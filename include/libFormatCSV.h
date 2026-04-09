/*
TrigaServer is a software for GNU operating system to get the real-time
values of the Nuclear Reator Triga IPR-R1 and share in network.
Copyright (C) 2023-2026 Thalles Campagnani

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

//libFormatCSV.h
#ifndef LIB_FORMAT_CSV_H
#define LIB_FORMAT_CSV_H

#include <string>

namespace FormatCSV {

    /**
     * @brief Gera o cabeçalho completo do arquivo CSV, substituindo os marcadores de canal e tipo dinamicamente.
     * @return std::string contendo a linha de cabeçalho terminada em \n
     */
    std::string generateCSVHeader();

    /**
     * @brief Gera o template de formatação do CSV (apenas os tipos de dados), seguindo a mesma ordem do cabeçalho.
     * @return std::string contendo a linha de template terminada em \n
     */
    std::string generateCSVTemplate();

}

#endif // LIB_FORMAT_CSV_H