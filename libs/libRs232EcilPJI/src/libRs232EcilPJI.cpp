/*
libRs232EcilPJI is a library to communicate with the ECIL PJ-I temperature
indicator using the Jotta serial protocol on RS-232C on a GNU operating system.
Copyright (C) 2026 Thalles Campagnani

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <libRs232EcilPJI.h>

#include <libserialport.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

namespace {

constexpr int kBaud           = 9600;
constexpr int kFirstByteMs    = 250;
constexpr int kInterByteMs    = 20;
constexpr size_t kMaxFrame    = 128;
constexpr uint16_t kIxMax     = 0x24;

uint8_t xor_bytes(const uint8_t* data, size_t n)
{
    uint8_t x = 0;
    for (size_t i = 0; i < n; ++i)
        x ^= data[i];
    return x;
}

uint16_t le16(uint8_t lo, uint8_t hi)
{
    return static_cast<uint16_t>(lo) | (static_cast<uint16_t>(hi) << 8);
}

float apply_decimal(int16_t raw, int dp)
{
    float scale = 1.f;
    const int n = (dp < 0) ? 0 : (dp > 4 ? 4 : dp);
    for (int i = 0; i < n; ++i)
        scale *= 10.f;
    return static_cast<float>(raw) / scale;
}

float fmt1(uint16_t raw, float low, float high)
{
    if (high == low)
        return low;
    return low + (high - low) * (static_cast<float>(raw) / 65535.f);
}

bool known_index(uint8_t ix)
{
    return (ix <= kIxMax) && ((ix & 1u) == 0);
}

void decode_status(PJI_DATA& d)
{
    if (d.status1 < 0 || d.status2 < 0)
        return;
    const int s1 = d.status1;
    const int s2 = d.status2;
    d.raiz_quadrada      = (s1 >> 2) & 1;
    d.unidade_F          = (s1 >> 4) & 1;
    d.escrita_habilitada = (s1 >> 5) & 1;
    d.det_min            = (s1 >> 6) & 1;
    d.det_max            = (s1 >> 7) & 1;
    d.pv_fora_faixa      = s2 & 1;
    d.entrada_nao_linear = (s2 >> 1) & 1;
    d.alarme1_cond       = (s2 >> 3) & 3;
    d.alarme2_cond       = (s2 >> 5) & 3;
    d.entrada_aberta     = (s2 >> 7) & 1;
}

void convert_engineering(PJI_DATA& d, const uint16_t raw[], const bool has[])
{
    const int dp = has[0x08] ? static_cast<int>(raw[0x08]) : 0;
    if (has[0x08])
        d.ponto_decimal = dp;

    if (has[0x0A])
        d.escala_alta_hw = apply_decimal(static_cast<int16_t>(raw[0x0A]), dp);
    if (has[0x0C])
        d.escala_baixa_hw = apply_decimal(static_cast<int16_t>(raw[0x0C]), dp);
    if (has[0x0E])
        d.escala_alta_trab = apply_decimal(static_cast<int16_t>(raw[0x0E]), dp);
    if (has[0x10])
        d.escala_baixa_trab = apply_decimal(static_cast<int16_t>(raw[0x10]), dp);

    const bool have_span = has[0x0E] && has[0x10];
    const float high = have_span ? d.escala_alta_trab : 100.f;
    const float low  = have_span ? d.escala_baixa_trab : 0.f;

    auto as_fmt1 = [&](uint8_t ix, float& out) {
        if (has[ix])
            out = fmt1(raw[ix], low, high);
    };

    if (has[0x00]) {
        d.PV_raw = raw[0x00];
        d.PV     = fmt1(raw[0x00], low, high);
    }
    as_fmt1(0x02, d.PV_max);
    as_fmt1(0x04, d.PV_min);
    if (has[0x06])
        d.tipo_entrada = raw[0x06];
    if (has[0x12]) {
        d.status1 = raw[0x12] & 0xFF;
        d.status2 = (raw[0x12] >> 8) & 0xFF;
        decode_status(d);
    }
    as_fmt1(0x14, d.sp_alarme1);
    as_fmt1(0x16, d.rearme_alarme1);
    if (has[0x18])
        d.chave_alarme1 = raw[0x18];
    as_fmt1(0x1A, d.sp_alarme2);
    as_fmt1(0x1C, d.rearme_alarme2);
    if (has[0x1E])
        d.chave_alarme2 = raw[0x1E];
    as_fmt1(0x20, d.pv_emergencia);
    if (has[0x22])
        d.n_medias = raw[0x22];
    if (has[0x24])
        d.trava_teclado = raw[0x24];
}

const uint8_t kTableOrder[] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10,
    0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24
};

bool parse_i_payload(const std::vector<uint8_t>& frame, uint16_t raw[], bool has[])
{
    if (frame.size() < 4)
        return false;

    const size_t payload = frame.size() - 2; // sem CE e sem CHK
    const uint8_t* p = frame.data() + 1;

    const bool looks_indexed =
        (payload % 3 == 0) && payload >= 3 && known_index(p[0]);

    if (looks_indexed) {
        for (size_t i = 0; i + 2 < payload; i += 3) {
            const uint8_t ix = p[i];
            if (ix > kIxMax)
                continue;
            raw[ix] = le16(p[i + 1], p[i + 2]);
            has[ix] = true;
        }
        return true;
    }

    if (payload % 2 != 0)
        return false;

    const size_t n = payload / 2;
    const size_t ncopy = n < (sizeof(kTableOrder) / sizeof(kTableOrder[0]))
                             ? n
                             : (sizeof(kTableOrder) / sizeof(kTableOrder[0]));
    for (size_t i = 0; i < ncopy; ++i) {
        const uint8_t ix = kTableOrder[i];
        raw[ix] = le16(p[i * 2], p[i * 2 + 1]);
        has[ix] = true;
    }
    return ncopy > 0;
}

} // namespace

const char* pji_tipo_entrada_nome(int codigo)
{
    switch (codigo) {
    case 0x00: return "T-ANSI";
    case 0x01: return "T-DIN";
    case 0x02: return "J-ANSI";
    case 0x03: return "J-DIN";
    case 0x04: return "E";
    case 0x05: return "K";
    case 0x06: return "S"; // não impresso na tabela 8.5; ordem da tabela 01
    case 0x07: return "R";
    case 0x08: return "B";
    case 0x09: return "W5";
    case 0x0A: return "RTD-DIN";
    case 0x0B: return "mA";
    case 0x0C: return "V";
    default:   return "?";
    }
}

struct libRs232EcilPJI_private {
    std::string portname;
    int address = 1;
    sp_port* port = nullptr;
    PJI_DATA data;
    bool flagNotConnected = true;
};

void libRs232EcilPJI_license()
{
    std::cout << "libRs232EcilPJI Copyright (C) 2026 Thalles Campagnani" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cout << "This is free software, and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the library." << std::endl << std::endl;
}

static void close_port(libRs232EcilPJI_private* p)
{
    if (!p->port)
        return;
    sp_close(p->port);
    sp_free_port(p->port);
    p->port = nullptr;
}

libRs232EcilPJI::libRs232EcilPJI(std::string portname, int address)
{
    this->_p = new libRs232EcilPJI_private;
    this->_p->portname = std::move(portname);
    if (address < 1)
        address = 1;
    if (address > 32)
        address = 32;
    this->_p->address = address;
    tryConnect();
}

libRs232EcilPJI::~libRs232EcilPJI()
{
    close_port(this->_p);
    delete this->_p;
}

std::string libRs232EcilPJI::get_portname() { return this->_p->portname; }
int         libRs232EcilPJI::get_address()  { return this->_p->address; }

std::string libRs232EcilPJI::stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg)
{
    std::string msg = "ERROR in libRs232EcilPJI::" + functionName +
                      " at " + get_portname() +
                      " addr=" + std::to_string(get_address()) +
                      "\n\tError type: " + errorMsg;
    if (!exptionMsg.empty())
        msg += "\n\tError code: " + exptionMsg;
    msg += "\n";
    return msg;
}

bool libRs232EcilPJI::tryConnect()
{
    close_port(this->_p);
    this->_p->flagNotConnected = true;

    sp_port* port = nullptr;
    sp_return ret = sp_get_port_by_name(this->_p->portname.c_str(), &port);
    if (ret != SP_OK || !port) {
        std::cerr << stdErrorMsg("tryConnect()", "Failed to find serial port",
                                 sp_last_error_message());
        this->_p->flagNotConnected = true;
        return false;
    }

    ret = sp_open(port, SP_MODE_READ_WRITE);
    if (ret != SP_OK) {
        std::cerr << stdErrorMsg("tryConnect()", "Failed to open serial port",
                                 sp_last_error_message());
        sp_free_port(port);
        return false;
    }

    if (sp_set_baudrate(port, kBaud) != SP_OK ||
        sp_set_bits(port, 8) != SP_OK ||
        sp_set_parity(port, SP_PARITY_EVEN) != SP_OK ||
        sp_set_stopbits(port, 1) != SP_OK ||
        sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE) != SP_OK) {
        std::cerr << stdErrorMsg("tryConnect()", "Failed to configure 9600 8E1",
                                 sp_last_error_message());
        sp_close(port);
        sp_free_port(port);
        return false;
    }

    sp_flush(port, SP_BUF_BOTH);
    this->_p->port = port;
    this->_p->flagNotConnected = false;
    std::cout << "Connection successful to: " << get_portname()
              << " (RS-232C 9600 8E1, addr " << get_address() << ")" << std::endl;
    return true;
}

static std::vector<uint8_t> read_frame(sp_port* port)
{
    std::vector<uint8_t> frame;
    frame.reserve(64);
    int timeout = kFirstByteMs;
    while (frame.size() < kMaxFrame) {
        uint8_t b = 0;
        const int n = sp_blocking_read(port, &b, 1, timeout);
        if (n == 1) {
            frame.push_back(b);
            timeout = kInterByteMs;
            continue;
        }
        break;
    }
    return frame;
}

PJI_DATA libRs232EcilPJI::get_all()
{
    auto fail = [&](int state, const char* fn, const std::string& msg, const std::string& extra = "") {
        std::cerr << stdErrorMsg(fn, msg, extra);
        this->_p->data.STATE = state;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    };

    if (!this->_p->port || this->_p->flagNotConnected)
        return fail(2, "get_all()", "Serial port is not open", "");

    uint8_t cmd[3];
    cmd[0] = static_cast<uint8_t>('I');
    cmd[1] = static_cast<uint8_t>(this->_p->address);
    cmd[2] = xor_bytes(cmd, 2);

    sp_flush(this->_p->port, SP_BUF_INPUT);
    const int written = sp_blocking_write(this->_p->port, cmd, 3, 100);
    if (written != 3) {
        return fail(1, "get_all()", "Failed to write command I",
                    sp_last_error_message());
    }
    sp_drain(this->_p->port);

    std::vector<uint8_t> frame = read_frame(this->_p->port);
    if (frame.size() >= 3 && frame[0] == cmd[0] && frame[1] == cmd[1] && frame[2] == cmd[2])
        frame.erase(frame.begin(), frame.begin() + 3);

    if (frame.size() < 4)
        return fail(1, "get_all()", "Short or empty response to command I",
                    "bytes=" + std::to_string(frame.size()));

    if (xor_bytes(frame.data(), frame.size()) != 0)
        return fail(1, "get_all()", "Checksum mismatch on command I response", "");

    if (frame[0] != static_cast<uint8_t>(this->_p->address))
        return fail(1, "get_all()", "Address mismatch on command I response",
                    "got=" + std::to_string(frame[0]));

    uint16_t raw[kIxMax + 1]{};
    bool has[kIxMax + 1]{};
    if (!parse_i_payload(frame, raw, has))
        return fail(1, "get_all()", "Could not parse command I payload",
                    "bytes=" + std::to_string(frame.size()));

    convert_engineering(this->_p->data, raw, has);
    this->_p->data.STATE = 0;
    this->_p->data.TIME = std::chrono::system_clock::now();
    return this->_p->data;
}
