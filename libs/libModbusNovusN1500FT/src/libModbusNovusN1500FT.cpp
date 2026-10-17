/*
libModbusNovusN1500FT is a library to communicate with the NOVUS N1500FT
flow indicator using MODBUS-RTU (RS-485) on a GNU operating system.
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

#include <libModbusNovusN1500FT.h>

#include <modbus/modbus-rtu.h>
#include <modbus/modbus.h>

#include <cerrno>
#include <cstring>
#include <iostream>

namespace {

constexpr int kTimeoutUs = 200000; // resposta ≤ 100 ms (manual)

std::string regsToText(uint16_t r0, uint16_t r1, uint16_t r2)
{
    char buf[7];
    buf[0] = static_cast<char>((r0 >> 8) & 0xFF);
    buf[1] = static_cast<char>(r0 & 0xFF);
    buf[2] = static_cast<char>((r1 >> 8) & 0xFF);
    buf[3] = static_cast<char>(r1 & 0xFF);
    buf[4] = static_cast<char>((r2 >> 8) & 0xFF);
    buf[5] = static_cast<char>(r2 & 0xFF);
    buf[6] = '\0';
    for (int i = 0; i < 6; ++i) {
        const unsigned char c = static_cast<unsigned char>(buf[i]);
        if (c < 0x20 || c > 0x7E)
            buf[i] = ' ';
    }
    int n = 6;
    while (n > 0 && buf[n - 1] == ' ')
        --n;
    buf[n] = '\0';
    return std::string(buf);
}

void decode_status(N1500FT_DATA& d)
{
    if (d.status1 >= 0) {
        d.alarme1 = d.status1 & 1;
        d.alarme2 = (d.status1 >> 1) & 1;
        d.alarme3 = (d.status1 >> 2) & 1;
        d.alarme4 = (d.status1 >> 3) & 1;
    }
    if (d.status2 >= 0) {
        d.aux_acima_max  = d.status2 & 1;
        d.aux_abaixo_min = (d.status2 >> 1) & 1;
        d.aux_aberta     = (d.status2 >> 2) & 1;
    }
    if (d.status3 >= 0) {
        d.rele1           = d.status3 & 1;
        d.rele2           = (d.status3 >> 1) & 1;
        d.rele3           = (d.status3 >> 2) & 1;
        d.rele4           = (d.status3 >> 3) & 1;
        d.entrada_digital = (d.status3 >> 4) & 1;
    }
}

char normalize_parity(char parity)
{
    if (parity == 'n') return 'N';
    if (parity == 'e') return 'E';
    if (parity == 'o') return 'O';
    return parity;
}

} // namespace

struct libModbusNovusN1500FT_private {
    std::string portname;
    int address = 1;
    int baud    = 9600;
    char parity = 'N';
    modbus_t* ctx = nullptr;
    N1500FT_DATA data;
    bool flagNotConnected = true;
};

void libModbusNovusN1500FT_license()
{
    std::cout << "libModbusNovusN1500FT Copyright (C) 2026 Thalles Campagnani" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cout << "This is free software, and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the library." << std::endl << std::endl;
}

const char* n1500ft_tipo_entrada_nome(int codigo)
{
    switch (codigo) {
    case 0: return "4-20mA";
    case 1: return "NPN/tensao";
    case 2: return "PNP";
    case 3: return "contato";
    case 4: return "pickup";
    default: return "?";
    }
}

const char* n1500ft_unidade_tempo_nome(int codigo)
{
    switch (codigo) {
    case 0: return "s";
    case 1: return "m";
    case 2: return "h";
    case 3: return "d";
    default: return "?";
    }
}

bool libModbusNovusN1500FT::tryConnect()
{
    this->_p->ctx = modbus_new_rtu(this->_p->portname.c_str(),
                                   this->_p->baud,
                                   this->_p->parity, 8, 1);

    if (this->_p->ctx == nullptr) {
        std::cerr << stdErrorMsg("tryConnect()", "Failed to create Modbus context",
                                 modbus_strerror(errno));
        this->_p->flagNotConnected = true;
        return true;
    }

    modbus_set_slave(this->_p->ctx, this->_p->address);
    modbus_set_response_timeout(this->_p->ctx, 0, kTimeoutUs);

    if (modbus_connect(this->_p->ctx) == -1) {
        std::cerr << stdErrorMsg("tryConnect()", "Failed to connect to Modbus device:",
                                 modbus_strerror(errno));
        modbus_close(this->_p->ctx);
        // modbus_free(this->_p->ctx); // Bug na libmodbus causa falha de segmentação
        this->_p->flagNotConnected = true;
        return true;
    }

    std::cout << "Connection successful to: " << get_portname()
              << " addr=" << get_address() << std::endl;
    this->_p->flagNotConnected = false;
    return false;
}

libModbusNovusN1500FT::libModbusNovusN1500FT(std::string portname, int address,
                                             int baud, char parity)
{
    this->_p = new libModbusNovusN1500FT_private;
    this->_p->portname = portname;
    this->_p->address  = address;
    this->_p->baud     = baud;
    this->_p->parity   = normalize_parity(parity);
    tryConnect();
}

libModbusNovusN1500FT::~libModbusNovusN1500FT()
{
    if (this->_p) {
        if (this->_p->ctx) {
            modbus_close(this->_p->ctx);
            // modbus_free(this->_p->ctx); // Bug na libmodbus causa falha de segmentação
        }
        delete this->_p;
    }
}

std::string libModbusNovusN1500FT::get_portname() { return this->_p->portname; }
int         libModbusNovusN1500FT::get_address()  { return this->_p->address; }
int         libModbusNovusN1500FT::get_baud()     { return this->_p->baud; }
char        libModbusNovusN1500FT::get_parity()   { return this->_p->parity; }

float libModbusNovusN1500FT::conv2RegsToFloat(uint16_t high, uint16_t low)
{
    // Manual: word high, depois word low. IEEE-754 single. Host little-endian.
    uint16_t words[2] = {low, high};
    float value = 0.f;
    std::memcpy(&value, words, sizeof(value));
    return value;
}

std::string libModbusNovusN1500FT::stdErrorMsg(std::string functionName,
                                               std::string errorMsg,
                                               std::string exptionMsg)
{
    std::string msg = "ERROR in libModbusNovusN1500FT::" + functionName +
                      " at " + get_portname() +
                      " addr=" + std::to_string(get_address()) +
                      "\n\tError type: " + errorMsg;
    if (!exptionMsg.empty())
        msg += "\n\tError code: " + exptionMsg;
    msg += "\n";
    return msg;
}

N1500FT_DATA libModbusNovusN1500FT::get_all()
{
    if (!this->_p->ctx || this->_p->flagNotConnected) {
        std::cerr << stdErrorMsg("get_all()", "Modbus context does not exist", "");
        this->_p->data.STATE = 2;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    }

    uint16_t proc[12];
    if (modbus_read_registers(this->_p->ctx, 0, 12, proc) == -1) {
        std::cerr << stdErrorMsg("get_all()", "Failed to read process registers",
                                 modbus_strerror(errno));
        this->_p->data.STATE = 1;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    }

    this->_p->data.vazao_instantanea = conv2RegsToFloat(proc[0], proc[1]);
    this->_p->data.vazao_total       = conv2RegsToFloat(proc[2], proc[3]);
    this->_p->data.vazao_total_nr    = conv2RegsToFloat(proc[4], proc[5]);
    this->_p->data.entrada_aux       = conv2RegsToFloat(proc[6], proc[7]);
    this->_p->data.vazao_inst_max    = conv2RegsToFloat(proc[8], proc[9]);
    this->_p->data.vazao_inst_min    = conv2RegsToFloat(proc[10], proc[11]);

    uint16_t units[8];
    if (modbus_read_registers(this->_p->ctx, 112, 8, units) == -1) {
        std::cerr << stdErrorMsg("get_all()", "Failed to read unit registers",
                                 modbus_strerror(errno));
        this->_p->data.STATE = 1;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    }

    this->_p->data.unidade_vazao         = regsToText(units[0], units[1], units[2]);
    this->_p->data.unidade_total         = regsToText(units[3], units[4], units[5]);
    this->_p->data.unidade_vazao_codigo  = static_cast<int>(units[6]);
    this->_p->data.tipo_entrada          = static_cast<int>(units[7]);

    uint16_t ident[4];
    if (modbus_read_registers(this->_p->ctx, 600, 4, ident) == -1) {
        std::cerr << stdErrorMsg("get_all()", "Failed to read identity registers",
                                 modbus_strerror(errno));
        this->_p->data.STATE = 1;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    }

    this->_p->data.serial_hi  = static_cast<int>(ident[0]);
    this->_p->data.serial_lo  = static_cast<int>(ident[1]);
    this->_p->data.firmware   = static_cast<int>(ident[2]);
    this->_p->data.product_id = static_cast<int>(ident[3]);

    uint16_t status[3];
    if (modbus_read_registers(this->_p->ctx, 628, 3, status) == -1) {
        std::cerr << stdErrorMsg("get_all()", "Failed to read status registers",
                                 modbus_strerror(errno));
        this->_p->data.STATE = 1;
        this->_p->data.TIME = std::chrono::system_clock::now();
        return this->_p->data;
    }

    this->_p->data.status1 = static_cast<int>(status[0]);
    this->_p->data.status2 = static_cast<int>(status[1]);
    this->_p->data.status3 = static_cast<int>(status[2]);
    decode_status(this->_p->data);

    this->_p->data.STATE = 0;
    this->_p->data.TIME = std::chrono::system_clock::now();
    return this->_p->data;
}
