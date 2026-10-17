/*
libRs232EcilPJI is a library to communicate with the ECIL PJ-I temperature
indicator using the Jotta serial protocol on RS-232C on a GNU operating system.
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

struct PJI_DATA
{
    int STATE = -1; // Status do PJ-I:
                    // -1 = Nenhuma tentativa de conexão realizada ainda.
                    //  0 = Valores lidos com sucesso.
                    //  1 = Erro ao ler dados.
                    //  2 = Erro de porta serial (dispositivo ausente ou fechada).
    std::chrono::system_clock::time_point TIME;

    float PV                 = -123456; // Variável de processo (unidade de engenharia)
    float PV_max             = -123456;
    float PV_min             = -123456;
    int   PV_raw             = -1;      // 0000H..FFFFH (formato 1)

    int   tipo_entrada       = -1;      // 0..12, ver pji_tipo_entrada_nome()
    int   ponto_decimal      = -1;      // 0..4
    float escala_alta_hw     = -123456;
    float escala_baixa_hw    = -123456;
    float escala_alta_trab   = -123456;
    float escala_baixa_trab  = -123456;

    int   status1            = -1;
    int   status2            = -1;
    int   raiz_quadrada      = -1;      // status1 bit 2
    int   unidade_F          = -1;      // status1 bit 4: 0=°C, 1=°F
    int   escrita_habilitada = -1;      // status1 bit 5
    int   det_min            = -1;      // status1 bit 6
    int   det_max            = -1;      // status1 bit 7
    int   pv_fora_faixa      = -1;      // status2 bit 0
    int   entrada_nao_linear = -1;      // status2 bit 1
    int   alarme1_cond       = -1;      // status2 bits 4:3
    int   alarme2_cond       = -1;      // status2 bits 6:5
    int   entrada_aberta     = -1;      // status2 bit 7

    float sp_alarme1         = -123456;
    float rearme_alarme1     = -123456;
    int   chave_alarme1      = -1;      // 0 inibido, 1 proc/aberta, 2 proc, 3 aberta
    float sp_alarme2         = -123456;
    float rearme_alarme2     = -123456;
    int   chave_alarme2      = -1;
    float pv_emergencia      = -123456;
    int   n_medias           = -1;
    int   trava_teclado      = -1;
};

// Coluna 0 = Header, coluna 1 = Tipo, coluna 2 = Unidade
inline std::vector<std::vector<std::string>> PJI_DATA_MATRIX = {
    {"PJI_%s_STATE",            "%d", "estado"},
    {"PJI_%s_TIME_Y",           "%d", "data/hora"},
    {"PJI_%s_TIME_Mo",          "%d", "data/hora"},
    {"PJI_%s_TIME_D",           "%d", "data/hora"},
    {"PJI_%s_TIME_H",           "%d", "data/hora"},
    {"PJI_%s_TIME_Mi",          "%d", "data/hora"},
    {"PJI_%s_TIME_S",           "%d", "data/hora"},
    {"PJI_%s_TIME_MS",          "%d", "data/hora"},
    {"PJI_%s_PV",               "%f", "UE"},
    {"PJI_%s_PV_max",           "%f", "UE"},
    {"PJI_%s_PV_min",           "%f", "UE"},
    {"PJI_%s_PV_raw",           "%d", "counts"},
    {"PJI_%s_tipo_entrada",     "%d", "codigo"},
    {"PJI_%s_ponto_decimal",    "%d", "digitos"},
    {"PJI_%s_escala_alta_hw",   "%f", "UE"},
    {"PJI_%s_escala_baixa_hw",  "%f", "UE"},
    {"PJI_%s_escala_alta_trab", "%f", "UE"},
    {"PJI_%s_escala_baixa_trab","%f", "UE"},
    {"PJI_%s_status1",          "%d", "bits"},
    {"PJI_%s_status2",          "%d", "bits"},
    {"PJI_%s_raiz_quadrada",    "%d", "logico"},
    {"PJI_%s_unidade_F",        "%d", "logico"},
    {"PJI_%s_escrita_habilitada","%d","logico"},
    {"PJI_%s_det_min",          "%d", "logico"},
    {"PJI_%s_det_max",          "%d", "logico"},
    {"PJI_%s_pv_fora_faixa",    "%d", "logico"},
    {"PJI_%s_entrada_nao_linear","%d","logico"},
    {"PJI_%s_alarme1_cond",     "%d", "codigo"},
    {"PJI_%s_alarme2_cond",     "%d", "codigo"},
    {"PJI_%s_entrada_aberta",   "%d", "logico"},
    {"PJI_%s_sp_alarme1",       "%f", "UE"},
    {"PJI_%s_rearme_alarme1",   "%f", "UE"},
    {"PJI_%s_chave_alarme1",    "%d", "codigo"},
    {"PJI_%s_sp_alarme2",       "%f", "UE"},
    {"PJI_%s_rearme_alarme2",   "%f", "UE"},
    {"PJI_%s_chave_alarme2",    "%d", "codigo"},
    {"PJI_%s_pv_emergencia",    "%f", "UE"},
    {"PJI_%s_n_medias",         "%d", "leituras"},
    {"PJI_%s_trava_teclado",    "%d", "codigo"}
};

const char* pji_tipo_entrada_nome(int codigo);

void libRs232EcilPJI_license();

struct libRs232EcilPJI_private;

class libRs232EcilPJI {
public:
    libRs232EcilPJI(std::string portname, int address = 1);
    ~libRs232EcilPJI();

    bool tryConnect();

    std::string get_portname();
    int         get_address();

    PJI_DATA get_all();

private:
    libRs232EcilPJI_private* _p;

    std::string stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg);
};
