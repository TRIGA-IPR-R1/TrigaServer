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
//libFormatCSV.cpp

#include "libFormatCSV.h"
#include <vector>
#include <cstdio>

#include <libModbusSystematomSPU.h>
#include <libOpcTrigaPLC.h>

namespace FormatCSV {

    // --- Funções Privadas ---
    namespace {
        std::string formatSPU(const std::string& format, const std::string& channel) {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), format.c_str(), channel.c_str());
            return std::string(buffer);
        }

        std::string formatPLC(const std::string& format, const char* type) {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), format.c_str(), type);
            return std::string(buffer);
        }
    }

    // --- Funções Públicas ---

    std::string generateCSVHeader() {
        std::string header = "";

        // 1. PLC_CONV
        for (const auto& row : PLC_DATA_MATRIX) {
            header += formatPLC(row[0], "CONV") + ";";
        }
        
        // 2. PLC_ORIG
        for (const auto& row : PLC_DATA_MATRIX) {
            header += formatPLC(row[0], "ORIG") + ";";
        }
        
        // 3. SPU_CHA
        for (const auto& row : SPU_DATA_MATRIX) {
            header += formatSPU(row[0], "ChA") + ";";
        }
        
        // 4. SPU_CHB
        for (const auto& row : SPU_DATA_MATRIX) {
            header += formatSPU(row[0], "ChB") + ";";
        }

        header += "\n";
        return header;
    }

    std::string generateCSVTemplate() {
        std::string tmpl = "";

        // 1. PLC_CONV
        for (const auto& row : PLC_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        
        // 2. PLC_ORIG
        for (const auto& row : PLC_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        
        // 3. SPU_CHA
        for (const auto& row : SPU_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        
        // 4. SPU_CHB
        for (const auto& row : SPU_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }

        tmpl += "\n";
        return tmpl;
    }

} // Fim do namespace FormatCSV