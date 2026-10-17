/*
libModbusNovusN1500FT is a library to communicate with the NOVUS N1500FT
flow indicator using MODBUS-RTU (RS-485) on a GNU operating system.
Copyright (C) 2026  Thalles Campagnani

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

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

struct N1500FT_DATA
{
    int STATE = -1; // Status do N1500FT:
                    // -1 = Nenhuma tentativa de conexão realizada ainda.
                    //  0 = Valores lidos com sucesso.
                    //  1 = Erro ao ler dados.
                    //  2 = Erro de contexto modbus (dispositivo USB ausente).
    std::chrono::system_clock::time_point TIME;

    float vazao_instantanea = -123456; // regs 0-1, IEEE-754
    float vazao_total       = -123456; // regs 2-3
    float vazao_total_nr    = -123456; // regs 4-5, não resetável
    float entrada_aux       = -123456; // regs 6-7, 4-20 mA auxiliar
    float vazao_inst_max    = -123456; // regs 8-9
    float vazao_inst_min    = -123456; // regs 10-11

    std::string unidade_vazao;         // regs 112-114, 6 ASCII (último = s/m/h/d)
    std::string unidade_total;         // regs 115-117, 6 ASCII
    int unidade_vazao_codigo = -1;     // reg 118, 0=s 1=m 2=h 3=d
    int tipo_entrada         = -1;     // reg 119, 0..4, ver n1500ft_tipo_entrada_nome()

    int serial_hi  = -1;               // reg 600, 4 primeiros dígitos
    int serial_lo  = -1;               // reg 601, 4 últimos dígitos
    int firmware   = -1;               // reg 602, 100 = 1.00
    int product_id = -1;               // reg 603, deve ser 80

    int status1 = -1;                  // reg 628
    int status2 = -1;                  // reg 629
    int status3 = -1;                  // reg 630

    int alarme1 = -1;                  // status1 bit 0
    int alarme2 = -1;
    int alarme3 = -1;
    int alarme4 = -1;

    int aux_acima_max  = -1;           // status2 bit 0
    int aux_abaixo_min = -1;           // status2 bit 1
    int aux_aberta     = -1;           // status2 bit 2

    int rele1           = -1;          // status3 bit 0
    int rele2           = -1;
    int rele3           = -1;
    int rele4           = -1;
    int entrada_digital = -1;          // status3 bit 4
};

// Coluna 0 = Header, coluna 1 = Tipo, coluna 2 = Unidade
inline std::vector<std::vector<std::string>> N1500FT_DATA_MATRIX = {
    {"N1500FT_%s_STATE",               "%d", "estado"},
    {"N1500FT_%s_TIME_Y",              "%d", "data/hora"},
    {"N1500FT_%s_TIME_Mo",             "%d", "data/hora"},
    {"N1500FT_%s_TIME_D",              "%d", "data/hora"},
    {"N1500FT_%s_TIME_H",              "%d", "data/hora"},
    {"N1500FT_%s_TIME_Mi",             "%d", "data/hora"},
    {"N1500FT_%s_TIME_S",              "%d", "data/hora"},
    {"N1500FT_%s_TIME_MS",             "%d", "data/hora"},
    {"N1500FT_%s_vazao_instantanea",   "%f", "UE"},
    {"N1500FT_%s_vazao_total",         "%f", "UE"},
    {"N1500FT_%s_vazao_total_nr",      "%f", "UE"},
    {"N1500FT_%s_entrada_aux",         "%f", "UE"},
    {"N1500FT_%s_vazao_inst_max",      "%f", "UE"},
    {"N1500FT_%s_vazao_inst_min",      "%f", "UE"},
    {"N1500FT_%s_unidade_vazao_codigo","%d", "codigo"},
    {"N1500FT_%s_tipo_entrada",        "%d", "codigo"},
    {"N1500FT_%s_serial_hi",           "%d", "digitos"},
    {"N1500FT_%s_serial_lo",           "%d", "digitos"},
    {"N1500FT_%s_firmware",            "%d", "codigo"},
    {"N1500FT_%s_product_id",          "%d", "codigo"},
    {"N1500FT_%s_status1",             "%d", "bits"},
    {"N1500FT_%s_status2",             "%d", "bits"},
    {"N1500FT_%s_status3",             "%d", "bits"},
    {"N1500FT_%s_alarme1",             "%d", "logico"},
    {"N1500FT_%s_alarme2",             "%d", "logico"},
    {"N1500FT_%s_alarme3",             "%d", "logico"},
    {"N1500FT_%s_alarme4",             "%d", "logico"},
    {"N1500FT_%s_aux_acima_max",       "%d", "logico"},
    {"N1500FT_%s_aux_abaixo_min",      "%d", "logico"},
    {"N1500FT_%s_aux_aberta",          "%d", "logico"},
    {"N1500FT_%s_rele1",               "%d", "logico"},
    {"N1500FT_%s_rele2",               "%d", "logico"},
    {"N1500FT_%s_rele3",               "%d", "logico"},
    {"N1500FT_%s_rele4",               "%d", "logico"},
    {"N1500FT_%s_entrada_digital",     "%d", "logico"}
};

const char* n1500ft_tipo_entrada_nome(int codigo);
const char* n1500ft_unidade_tempo_nome(int codigo);

void libModbusNovusN1500FT_license();

struct libModbusNovusN1500FT_private;

class libModbusNovusN1500FT {
public:
    // parity: 'N' nenhuma, 'E' par, 'O' ímpar. Stop bit fixo = 1 (manual).
    libModbusNovusN1500FT(std::string portname, int address = 1,
                          int baud = 9600, char parity = 'N');
    ~libModbusNovusN1500FT();

    bool tryConnect();

    std::string get_portname();
    int         get_address();
    int         get_baud();
    char        get_parity();

    N1500FT_DATA get_all();

private:
    libModbusNovusN1500FT_private* _p;

    std::string stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg);
    float conv2RegsToFloat(uint16_t high, uint16_t low);
};
