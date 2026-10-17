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
//TrigaServer.h
#ifndef TRIGA_SERVER
#define TRIGA_SERVER

#include <libModbusSystematomSPU.h>
#include <libOpcTrigaPLC.h>
#include <libRs232EcilPJI.h>

#include <array>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <boost/asio.hpp>
#include <json/json.h>
#include <string>
#include <vector>

inline constexpr int PJI_COUNT = 5;
inline constexpr const char* PJI_NAMES[PJI_COUNT] = {
    "Poco", "EntPri", "SaiPri", "EntSec", "SaiSec"
};

//#define TestMax

using boost::asio::ip::tcp;

struct ALL_DATA
{
    PLC_DATA PLC;
    SPU_DATA SPU_CHA;
    SPU_DATA SPU_CHB;
    PJI_DATA PJI[PJI_COUNT]; // Poco, EntPri, SaiPri, EntSec, SaiSec
};

struct int_TIME
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
};

class TrigaServer
{
    public:
        TrigaServer(std::string spu_sp1, 
                    std::string spu_sp2, 
                    std::string clp_adress, 
                    std::string clp_conv_file, 
                    int error_interval_plc,
                    int error_interval_spu,
                    std::array<std::string, PJI_COUNT> pji_ports,
                    std::array<int, PJI_COUNT> pji_addrs,
                    int error_interval_pji);
        ~TrigaServer();

        //Função que cria servidor TCP
        void createServer(int port);

        //Função para leitura de estados das readThreads
        std::vector<int> state();

        //Função para leitura dos valores de potência e periodo
        std::vector<float> readPP();

        // Temperatura dos cinco PJ-I (Poco, EntPri, SaiPri, EntSec, SaiSec)
        std::vector<float> readPJI();

    private:
        // Objetos para comunicação Modbus nas duas portas seriais
        libModbusSystematomSPU spuChA;
        libModbusSystematomSPU spuChB;
        libOpcTrigaPLC         plc;
        std::unique_ptr<libRs232EcilPJI> pjiDev[PJI_COUNT];

        // Variáveis para armezenar o endereço da SPU A e B.
        std::string adressSpuA;
        std::string adressSpuB;
        std::array<std::string, PJI_COUNT> adressPji;

        //Variáveis para armezenar o intervalo de reconexão em caso de erro
        int errorIntervalSPU;
        int errorIntervalPLC;
        int errorIntervalPJI;

        // Cabeçalho CSV com nome[unidade], gerado após ler o arquivo de calibração.
        std::string csvHeader;

        //Ponteiros inteligentes globais
        std::atomic<std::shared_ptr<SPU_DATA>> data_global_spuCh[2] = {std::make_shared<SPU_DATA>(),
                                                                       std::make_shared<SPU_DATA>()};
        std::atomic<std::shared_ptr<PLC_DATA>> data_global_plc      =  std::make_shared<PLC_DATA>();
        std::atomic<std::shared_ptr<PJI_DATA>> data_global_pji[PJI_COUNT] = {
            std::make_shared<PJI_DATA>(),
            std::make_shared<PJI_DATA>(),
            std::make_shared<PJI_DATA>(),
            std::make_shared<PJI_DATA>(),
            std::make_shared<PJI_DATA>()
        };

        // Função para gerar string JSON com todos os dados
        std::string genString(ALL_DATA all_data);

        //Função para converter time_point em struct int_TIME (usando pelo genJson)
        int_TIME decodeTime(std::chrono::system_clock::time_point t);

        //Função que lida com os clientes (recebe o valor de intervalo e cria uma thread para cada cliente)
        void handleTCPClients(int clientSocket);

        //Threads de leitura de hardware
        void startReadThreads();
        void readModbusRTU(libModbusSystematomSPU& spu);
        void readOpcTCP(libOpcTrigaPLC& plc);
        void readRs232PJI(int index);
        std::string formatPjiCsv(const PJI_DATA& data);
};

#endif
