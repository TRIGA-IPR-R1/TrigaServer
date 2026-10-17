/*
This is a test program for libRs232EcilPJI, a library to communicate with
the ECIL PJ-I using the Jotta serial protocol on RS-232C.
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

#include <libRs232EcilPJI.h>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    libRs232EcilPJI_license();
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <serial_port_name> <address_1_32> <qtd_of_reads>" << std::endl;
        return 1;
    }

    try {
        libRs232EcilPJI pji(argv[1], std::stoi(argv[2]));
        const int n = std::stoi(argv[3]);
        for (int i = 0; i < n; ++i) {
            PJI_DATA data = pji.get_all();
            std::cout << "STATE=" << data.STATE
                      << "  PV=" << data.PV
                      << (data.unidade_F == 1 ? " F" : " C")
                      << "  raw=" << data.PV_raw
                      << "  tipo=" << pji_tipo_entrada_nome(data.tipo_entrada)
                      << "  span=[" << data.escala_baixa_trab
                      << "," << data.escala_alta_trab << "]"
                      << "  AL1=" << data.alarme1_cond
                      << "  AL2=" << data.alarme2_cond
                      << "  aberta=" << data.entrada_aberta
                      << std::endl;
        }
    }
    catch (int) {
        return 2;
    }
    return 0;
}
