#include "libFormatCSV.h"
#include <vector>
#include <cstdio>

#include <libModbusSystematomSPU.h>
#include <libOpcTrigaPLC.h>

namespace FormatCSV {

    // --- Funções Privadas ---
    namespace {
        std::string formatSPU(const std::string& format, char channel) {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), format.c_str(), channel);
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
            header += formatSPU(row[0], 'ChA') + ";";
        }
        
        // 4. SPU_CHB
        for (const auto& row : SPU_DATA_MATRIX) {
            header += formatSPU(row[0], 'ChB') + ";";
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