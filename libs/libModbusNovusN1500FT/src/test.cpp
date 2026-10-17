/*
This is a test program for libModbusNovusN1500FT, a library to communicate
with the NOVUS N1500FT using MODBUS-RTU (RS-485).
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

#include <libModbusNovusN1500FT.h>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    libModbusNovusN1500FT_license();
    if (argc < 4 || argc > 6) {
        std::cerr << "Usage: " << argv[0]
                  << " <serial_port_name> <address_1_247> <qtd_of_reads>"
                  << " [baud=9600] [parity=N|E|O]" << std::endl;
        return 1;
    }

    const int baud = (argc >= 5) ? std::stoi(argv[4]) : 9600;
    const char parity = (argc >= 6) ? argv[5][0] : 'N';

    try {
        libModbusNovusN1500FT ft(argv[1], std::stoi(argv[2]), baud, parity);
        const int n = std::stoi(argv[3]);
        for (int i = 0; i < n; ++i) {
            N1500FT_DATA data = ft.get_all();
            std::cout << "STATE=" << data.STATE
                      << "  Q=" << data.vazao_instantanea
                      << " " << data.unidade_vazao
                      << "  tot=" << data.vazao_total
                      << " " << data.unidade_total
                      << "  totNR=" << data.vazao_total_nr
                      << "  aux=" << data.entrada_aux
                      << "  tipo=" << n1500ft_tipo_entrada_nome(data.tipo_entrada)
                      << "  fw=" << data.firmware
                      << "  AL=" << data.alarme1 << data.alarme2
                      << data.alarme3 << data.alarme4
                      << "  aberta=" << data.aux_aberta
                      << std::endl;
        }
    }
    catch (int) {
        return 2;
    }
    return 0;
}
