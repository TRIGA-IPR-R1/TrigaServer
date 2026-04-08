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