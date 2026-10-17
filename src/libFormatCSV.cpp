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
#include <libRs232EcilPJI.h>

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

        std::string fieldFromFormat(const std::string& format)
        {
            const std::string marker = "%s_";
            auto pos = format.find(marker);
            if (pos == std::string::npos) return "";
            return format.substr(pos + marker.size());
        }

        std::string withUnit(const std::string& name, const std::string& unit)
        {
            return name + "[" + unit + "]";
        }

        std::string plcHeaderUnit(const CONV_PLC& conv, const std::string& field, bool orig)
        {
            if (field == "STATE") return "estado";
            if (field.rfind("TIME_", 0) == 0) return "data/hora";
            if (orig) return field == "SVasPri" ? "mA" : "mV";
            return convUnit(conv, field);
        }
    }

    // --- Funções Públicas ---

    std::string generateCSVHeader(const CONV_PLC& conv) {
        std::string header = "";

        // 1. PLC_CONV — unidades do arquivo de calibração
        for (const auto& row : PLC_DATA_MATRIX) {
            const std::string field = fieldFromFormat(row[0]);
            header += withUnit(formatPLC(row[0], "CONV"),
                               plcHeaderUnit(conv, field, false)) + ";";
        }
        
        // 2. PLC_ORIG — leitura bruta (analógico em mV; vazão em mA)
        for (const auto& row : PLC_DATA_MATRIX) {
            const std::string field = fieldFromFormat(row[0]);
            header += withUnit(formatPLC(row[0], "ORIG"),
                               plcHeaderUnit(conv, field, true)) + ";";
        }
        
        // 3. SPU_CHA
        for (const auto& row : SPU_DATA_MATRIX) {
            const std::string unit = row.size() > 2 ? row[2] : "";
            header += withUnit(formatSPU(row[0], "ChA"), unit) + ";";
        }
        
        // 4. SPU_CHB
        for (const auto& row : SPU_DATA_MATRIX) {
            const std::string unit = row.size() > 2 ? row[2] : "";
            header += withUnit(formatSPU(row[0], "ChB"), unit) + ";";
        }

        // 5. PJ-I: Poco, EntPri, SaiPri, EntSec, SaiSec
        static const char* pjiNames[] = {"Poco", "EntPri", "SaiPri", "EntSec", "SaiSec"};
        for (const char* name : pjiNames) {
            for (const auto& row : PJI_DATA_MATRIX) {
                const std::string unit = row.size() > 2 ? row[2] : "";
                header += withUnit(formatSPU(row[0], name), unit) + ";";
            }
        }

        header += "\n";
        return header;
    }

    std::string generatePlcCSVTemplate() {
        std::string tmpl = "";
        for (const auto& row : PLC_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        return tmpl;
    }

    std::string generateSpuCSVTemplate() {
        std::string tmpl = "";
        for (const auto& row : SPU_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        return tmpl;
    }

    std::string generatePjiCSVTemplate() {
        std::string tmpl = "";
        for (const auto& row : PJI_DATA_MATRIX) {
            tmpl += row[1] + ";";
        }
        return tmpl;
    }

    std::string generateCSVTemplate() {
        std::string tmpl  = generatePlcCSVTemplate(); // PLC_CONV
        tmpl += generatePlcCSVTemplate();             // PLC_ORIG
        tmpl += generateSpuCSVTemplate();             // SPU_CHA
        tmpl += generateSpuCSVTemplate();             // SPU_CHB
        tmpl += generatePjiCSVTemplate();             // PJI_Poco
        tmpl += generatePjiCSVTemplate();             // PJI_EntPri
        tmpl += generatePjiCSVTemplate();             // PJI_SaiPri
        tmpl += generatePjiCSVTemplate();             // PJI_EntSec
        tmpl += generatePjiCSVTemplate();             // PJI_SaiSec
        tmpl += "\n";
        return tmpl;
    }

} // Fim do namespace FormatCSV