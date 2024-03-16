/*
TrigaServer is a software for GNU operating system to get the real-time
values of the Nuclear Reator Triga IPR-R1 and share in network.
Copyright (C) 2023-2024 Thalles Campagnani

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
//TrigaServer.cpp
#include "TrigaServer.h"

TrigaServer::TrigaServer(std::string spu_sp1,//SPU_CH_A serial port
                         std::string spu_sp2,//SPU_CH_B serial port
                         std::string clp_ip, //CLP IP
                         std::string clp_port) //CPL Port
                        :spuChA(spu_sp1),
                         spuChB(spu_sp2),
                         plc(clp_ip,clp_port)
{
}


TrigaServer::~TrigaServer() {
    for(auto& thread : clientThreads) {
        if(thread.joinable())
            thread.join();
    }
}

void TrigaServer::startReadThreads() // Método para Threads
{
    //Iniciar todas as treads necessárias
    std::thread spuChAThread(&TrigaServer::readModbusRTU, this, std::ref(spuChA));
    std::thread spuChBThread(&TrigaServer::readModbusRTU, this, std::ref(spuChB));
    std::thread plcThread   (&TrigaServer::readOpcTCP,    this, std::ref(plc));
    #ifdef TestMax
        for (int i = 0; i < 10; i++) {
            std::thread serverThread(&TrigaServer::handleTCPClients, this, false);
            serverThread.detach();
        }
    #endif

    //Esperar todas as threads terminarem (idelmente não deveria terminar)
    spuChAThread.join();
    spuChBThread.join();
    plcThread.join();
}

void TrigaServer::startServer(int port, bool sendJson) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "[startServer] Error opening socket" << std::endl;
        return;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[startServer] Error on binding" << std::endl;
        return;
    }

    listen(serverSocket, 5);
    std::cout << "[startServer] Server started on port " << port << std::endl;

    while(true) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            std::cerr << "[startServer] Error on accept" << std::endl;
            continue;
        }

        std::cout << "[startServer] Client connected" << std::endl;
        
        std::thread clientThread(&TrigaServer::handleTCPClients, this, clientSocket, sendJson);
        clientThread.detach();
    }
}

void TrigaServer::handleTCPClients(int clientSocket, bool sendJson)
{
    int timeout = 2; // 2 seconds timeout
    char buffer[1024];
    int n = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        std::cerr << "[handleTCPClients] Error receiving data" << std::endl;
        close(clientSocket);
        return;
    }

    // Parse received data (assuming it's a number)
    int interval = std::stoi(std::string(buffer, n));

    std::cout << "[handleTCPClients] Received interval: " << interval << "ms" << std::endl;

    if(sendJson)
    {
        // Create new thread
        std::thread([this, interval, clientSocket]()
        {
            while(true)
            {
                auto data_local_spuChA = std::shared_ptr <SPU_DATA> (new SPU_DATA);
                auto data_local_spuChB = std::shared_ptr <SPU_DATA> (new SPU_DATA);
                auto data_local_plc    = std::shared_ptr <PLC_DATA> (new PLC_DATA);

                data_local_spuChA = data_global_spuChA.load();
                data_local_spuChB = data_global_spuChB.load();
                data_local_plc = data_global_plc.load();

                ALL_DATA data;
                data.SPU_CHA = *data_local_spuChA;
                data.SPU_CHB = *data_local_spuChB;
                data.PLC    = *data_local_plc;
                
                std::string json = genJson(data);

                send(clientSocket, json.c_str(), json.size(), 0);

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        }).detach();
    }
    else
    {
        // Create new thread
        std::thread([this, interval, clientSocket]()
        {
            while(true)
            {
                auto data_local_spuChA = std::shared_ptr <SPU_DATA> (new SPU_DATA);
                auto data_local_spuChB = std::shared_ptr <SPU_DATA> (new SPU_DATA);
                auto data_local_plc    = std::shared_ptr <PLC_DATA> (new PLC_DATA);

                data_local_spuChA = data_global_spuChA.load();
                data_local_spuChB = data_global_spuChB.load();
                data_local_plc = data_global_plc.load();

                #ifndef TestMax
                    ALL_DATA data;
                    data.SPU_CHA = *data_local_spuChA;
                    data.SPU_CHB = *data_local_spuChB;
                    data.PLC     = *data_local_plc;
                    
                    // Convert ALL_DATA to a char pointer
                    char* buffer = reinterpret_cast<char*>(&data);
                    size_t size = sizeof(data);

                    // Send data
                    send(clientSocket, buffer, size, 0);

                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                #endif
            }
        }).detach();
    }
}

// Ler dados da SPU usando libModbusSystematomSPU
void TrigaServer::readModbusRTU(libModbusSystematomSPU& spu)
{
    while (true)
    {
        //Criação do ponteiro inteligente local
        auto data_local = std::shared_ptr <SPU_DATA> (new SPU_DATA);
        //Realizando leitura e salvando informação se foi lido
        *data_local  = spu.get_all();
        //Passar para o ponteiro inteligente global
        if (spu.get_portname() == "/dev/ttyF0") data_global_spuChA.store(data_local);
        else                                    data_global_spuChB.store(data_local);
    }
}

// Ler dados Modbus do PLC usando libModbusSiemensPLC
void TrigaServer::readOpcTCP(libOpcTrigaPLC& plc) 
{
    while (true)
    {
        auto data_local = std::shared_ptr <PLC_DATA> (new PLC_DATA);
        *data_local = plc.get_all();
        data_global_plc.store(data_local);
    }
}

const char * JSON_TEMPLATE = 
    "{\n\
        SPU_CHA: {\n\
            READ: %d,\n\
            N_DATA_FP: %f,\n\
            T_DATA_FP: %f,\n\
            F1_DATA_FP: %f,\n\
            F2_DATA_FP: %f,\n\
            F3_DATA_FP: %f,\n\
            EMR_N_THRESHOLD: %f,\n\
            WRN_N_THRESHOLD: %f,\n\
            EMR_T_THRESHOLD: %f,\n\
            WRN_T_THRESHOLD: %f,\n\
            EMR_N: %d,\n\
            WRN_N: %d,\n\
            EMR_T: %d,\n\
            WRN_T: %d,\n\
            R1: %d,\n\
            R2: %d,\n\
            R3: %d,\n\
            RDY: %d,\n\
            TEST: %d,\n\
            XXXX: %d\n\
        },\n\
        SPU_CHB: {\n\
            READ: %d,\n\
            N_DATA_FP: %f,\n\
            T_DATA_FP: %f,\n\
            F1_DATA_FP: %f,\n\
            F2_DATA_FP: %f,\n\
            F3_DATA_FP: %f,\n\
            EMR_N_THRESHOLD: %f,\n\
            WRN_N_THRESHOLD: %f,\n\
            EMR_T_THRESHOLD: %f,\n\
            WRN_T_THRESHOLD: %f,\n\
            EMR_N: %d,\n\
            WRN_N: %d,\n\
            EMR_T: %d,\n\
            WRN_T: %d,\n\
            R1: %d,\n\
            R2: %d,\n\
            R3: %d,\n\
            RDY: %d,\n\
            TEST: %d,\n\
            XXXX: %d\n\
        },\n\
        PLC: {\n\
            STATE: %d,\n\
            BarraReg: %f,\n\
            BarraCon: %f,\n\
            BarraSeg: %f,\n\
            CLogALog: %f,\n\
            CLogALin: %f,\n\
            CLogAPer: %f,\n\
            CParALin: %f,\n\
            CParALog: %f,\n\
            CParAPer: %f,\n\
            CLogARea: %f,\n\
            CLin: %f,\n\
            CPer: %f,\n\
            SRadAre: %f,\n\
            SRadEntPri: %f,\n\
            SRadPoc: %f,\n\
            SRadRes: %f,\n\
            SRadSaiSec: %f,\n\
            SRadAer: %f,\n\
            VasPri: %f,\n\
            SPt100Poco: %f,\n\
            SPt100EntPri: %f,\n\
            SPt100SaiPri: %f,\n\
            SPt100EntSec: %f,\n\
            SPt100SaiSec: %f,\n\
            STpPoc1: %f,\n\
            STpPoc2: %f,\n\
            STpLen: %f,\n\
            SConPoc: %f,\n\
            SConSaiPri: %f,\n\
        },\n\
    }\n";

std::string TrigaServer::genJson(ALL_DATA all_data) 
{
    char buffer[4096];
    sprintf(buffer, JSON_TEMPLATE, all_data.SPU_CHA.STATE,
            all_data.SPU_CHA.N_DATA_FP,
            all_data.SPU_CHA.T_DATA_FP,
            all_data.SPU_CHA.F1_DATA_FP,
            all_data.SPU_CHA.F2_DATA_FP,
            all_data.SPU_CHA.F3_DATA_FP,
            all_data.SPU_CHA.EMR_N_THRESHOLD,
            all_data.SPU_CHA.WRN_N_THRESHOLD,
            all_data.SPU_CHA.EMR_T_THRESHOLD,
            all_data.SPU_CHA.WRN_T_THRESHOLD,
            all_data.SPU_CHA.EMR_N,
            all_data.SPU_CHA.WRN_N,
            all_data.SPU_CHA.EMR_T,
            all_data.SPU_CHA.WRN_T,
            all_data.SPU_CHA.R1,
            all_data.SPU_CHA.R2,
            all_data.SPU_CHA.R3,
            all_data.SPU_CHA.RDY,
            all_data.SPU_CHA.TEST,
            all_data.SPU_CHA.XXXX,

            all_data.SPU_CHB.STATE,
            all_data.SPU_CHB.N_DATA_FP,
            all_data.SPU_CHB.T_DATA_FP,
            all_data.SPU_CHB.F1_DATA_FP,
            all_data.SPU_CHB.F2_DATA_FP,
            all_data.SPU_CHB.F3_DATA_FP,
            all_data.SPU_CHB.EMR_N_THRESHOLD,
            all_data.SPU_CHB.WRN_N_THRESHOLD,
            all_data.SPU_CHB.EMR_T_THRESHOLD,
            all_data.SPU_CHB.WRN_T_THRESHOLD,
            all_data.SPU_CHB.EMR_N,
            all_data.SPU_CHB.WRN_N,
            all_data.SPU_CHB.EMR_T,
            all_data.SPU_CHB.WRN_T,
            all_data.SPU_CHB.R1,
            all_data.SPU_CHB.R2,
            all_data.SPU_CHB.R3,
            all_data.SPU_CHB.RDY,
            all_data.SPU_CHB.TEST,
            all_data.SPU_CHB.XXXX,
            
            all_data.PLC.STATE,
            all_data.PLC.BarraReg,
            all_data.PLC.BarraCon,
            all_data.PLC.BarraSeg,
            all_data.PLC.CLogALog,
            all_data.PLC.CLogALin,
            all_data.PLC.CLogAPer,
            all_data.PLC.CParALin,
            all_data.PLC.CParALog,
            all_data.PLC.CParAPer,
            all_data.PLC.CLogARea,
            all_data.PLC.CLin,
            all_data.PLC.CPer,
            all_data.PLC.SRadAre,
            all_data.PLC.SRadEntPri,
            all_data.PLC.SRadPoc,
            all_data.PLC.SRadRes,
            all_data.PLC.SRadSaiSec,
            all_data.PLC.SRadAer,
            all_data.PLC.VasPri,
            all_data.PLC.SPt100Poco,
            all_data.PLC.SPt100EntPri,
            all_data.PLC.SPt100SaiPri,
            all_data.PLC.SPt100EntSec,
            all_data.PLC.SPt100SaiSec,
            all_data.PLC.STpPoc1,
            all_data.PLC.STpPoc2,
            all_data.PLC.STpLen,
            all_data.PLC.SConPoc,
            all_data.PLC.SConSaiPri
            );
    return buffer;
}