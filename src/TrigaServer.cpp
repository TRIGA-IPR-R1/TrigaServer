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

//Linha contendo o nome de todas as variáveis a serem enviadas (cabeçalho), no formato CSV
const char CSV_HEADER[] = 
"SPU_CHA_STATE;SPU_CHA_TIME_Y;SPU_CHA_TIME_Mo;SPU_CHA_TIME_D;SPU_CHA_TIME_H;SPU_CHA_TIME_Mi;SPU_CHA_TIME_S;SPU_CHA_TIME_MS;SPU_CHA_N_DATA_FP;SPU_CHA_T_DATA_FP;SPU_CHA_F1_DATA_FP;SPU_CHA_F2_DATA_FP;SPU_CHA_F3_DATA_FP;SPU_CHA_EMR_N_THRESHOLD;SPU_CHA_WRN_N_THRESHOLD;SPU_CHA_EMR_T_THRESHOLD;SPU_CHA_WRN_T_THRESHOLD;SPU_CHA_EMR_N;SPU_CHA_WRN_N;SPU_CHA_EMR_T;SPU_CHA_WRN_T;SPU_CHA_R1;SPU_CHA_R2;SPU_CHA_R3;SPU_CHA_RDY;SPU_CHA_TEST;SPU_CHA_XXXX;SPU_CHB_STATE;SPU_CHB_TIME_Y;SPU_CHB_TIME_Mo;SPU_CHB_TIME_D;SPU_CHB_TIME_H;SPU_CHB_TIME_Mi;SPU_CHB_TIME_S;SPU_CHB_TIME_MS;SPU_CHB_N_DATA_FP;SPU_CHB_T_DATA_FP;SPU_CHB_F1_DATA_FP;SPU_CHB_F2_DATA_FP;SPU_CHB_F3_DATA_FP;SPU_CHB_EMR_N_THRESHOLD;SPU_CHB_WRN_N_THRESHOLD;SPU_CHB_EMR_T_THRESHOLD;SPU_CHB_WRN_T_THRESHOLD;SPU_CHB_EMR_N;SPU_CHB_WRN_N;SPU_CHB_EMR_T;SPU_CHB_WRN_T;SPU_CHB_R1;SPU_CHB_R2;SPU_CHB_R3;SPU_CHB_RDY;SPU_CHB_TEST;SPU_CHB_XXXX;PLC_ORIG_STATE;PLC_ORIG_TIME_Y;PLC_ORIG_TIME_Mo;PLC_ORIG_TIME_D;PLC_ORIG_TIME_H;PLC_ORIG_TIME_Mi;PLC_ORIG_TIME_S;PLC_ORIG_TIME_MS;PLC_ORIG_BarraReg;PLC_ORIG_BarraCon;PLC_ORIG_BarraSeg;PLC_ORIG_CLogALog;PLC_ORIG_CLogALin;PLC_ORIG_CLogAPer;PLC_ORIG_CParALin;PLC_ORIG_CParALog;PLC_ORIG_CParAPer;PLC_ORIG_CLogARea;PLC_ORIG_CLin;PLC_ORIG_CPer;PLC_ORIG_SRadAre;PLC_ORIG_SRadEntPri;PLC_ORIG_SRadPoc;PLC_ORIG_SRadRes;PLC_ORIG_SRadSaiSec;PLC_ORIG_SRadAer;PLC_ORIG_SVasPri;PLC_CONV_STATE;PLC_CONV_TIME_Y;PLC_CONV_TIME_Mo;PLC_CONV_TIME_D;PLC_CONV_TIME_H;PLC_CONV_TIME_Mi;PLC_CONV_TIME_S;PLC_CONV_TIME_MS;PLC_CONV_BarraReg;PLC_CONV_BarraCon;PLC_CONV_BarraSeg;PLC_CONV_CLogALog;PLC_CONV_CLogALin;PLC_CONV_CLogAPer;PLC_CONV_CParALin;PLC_CONV_CParALog;PLC_CONV_CParAPer;PLC_CONV_CLogARea;PLC_CONV_CLin;PLC_CONV_CPer;PLC_CONV_SRadAre;PLC_CONV_SRadEntPri;PLC_CONV_SRadPoc;PLC_CONV_SRadRes;PLC_CONV_SRadSaiSec;PLC_CONV_SRadAer;PLC_CONV_SVasPri;\n";

//Linha contendo os valores de todas variáveis, no formato CSV
const char* CSV_TEMPLATE = 
"%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%d;%d;%d;%d;%d;%d;%d;%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;\n";

//Construtor da classe
TrigaServer::TrigaServer(std::string spu_sp1,//Caminho da porta serial da SPU_CH_A
                         std::string spu_sp2,//Caminho da porta serial da SPU_CH_B
                         std::string clp_adress, //IP do CLP
                         std::string clp_conv_file,//Caminho do arquivo de conversão das variáveis do CLP
                         int error_interval_plc,//Valor do intervalo de reconexão do CLP em caso de erro
                         int error_interval_spu)//Valor do intervalo de reconexão da SPU em caso de erro
                        :spuChA(spu_sp1),//Cria objeto do tipo libModbusSystematomSPU para SPU_CH_A passando o caminho spu_sp1
                         spuChB(spu_sp2),//Cria objeto do tipo libModbusSystematomSPU para SPU_CH_B passando o caminho spu_sp2
                         plc(clp_adress,clp_conv_file)//Cria objeto do tipo libOpcTrigaPLC passando IP e caminho do arquivo de conversão
{
    adressSpuA = spu_sp1;//Salva enderço da SPU_CH_A para uso posterior
    adressSpuB = spu_sp2;//Salva enderço da SPU_CH_B para uso posterior
    errorIntervalSPU = error_interval_plc;
    errorIntervalPLC = error_interval_spu;
    startReadThreads();
}

//Destrutor não faz nada
//Poderia ser implementado para encerrar as conexões gentilmente
TrigaServer::~TrigaServer() {}

//Método para verificar estado das conexões (usado para log)
std::vector<int> TrigaServer::state()
{
    auto localSpuA = data_global_spuCh[0].load();
    auto localSpuB = data_global_spuCh[1].load();
    auto localPlc = data_global_plc.load();

    return {localSpuA.get()->STATE,
            localSpuB.get()->STATE,
            localPlc.get()->STATE};
}

//Método para verificar leitura de potência e periodo de cada hardware (usado para log)
std::vector<float> TrigaServer::readPP()
{
    auto localSpuA = data_global_spuCh[0].load();
    auto localSpuB = data_global_spuCh[1].load();
    auto localPlc = data_global_plc.load();

    return {localSpuA.get()->N_DATA_FP,
            localSpuA.get()->T_DATA_FP,
            localSpuB.get()->N_DATA_FP,
            localSpuB.get()->T_DATA_FP,
            localPlc.get()->CLogALin,
            localPlc.get()->CLogALog,
            localPlc.get()->CLogAPer};
}

// Método para Threads de leitura dos hardware (aquisição de dados)
void TrigaServer::startReadThreads()
{
    //Iniciar todas as threads de leitura
    std::thread spuChAThread(&TrigaServer::readModbusRTU, this, std::ref(spuChA));
    std::thread spuChBThread(&TrigaServer::readModbusRTU, this, std::ref(spuChB));
    std::thread plcThread   (&TrigaServer::readOpcTCP,    this, std::ref(plc));

    //Teste de erro de concorrência de memória
    #ifdef TestMax
        for (int i = 0; i < 10; i++) {
            std::thread serverThread(&TrigaServer::handleTCPClients, this, false);
            serverThread.detach();
        }
    #endif

    //Desvincular das threads de leitura antes de encerrar a execução do método
    spuChAThread.detach();
    spuChBThread.detach();
    plcThread.detach();
}

//Método que inicia servidor que escuta novos clientes
//Esse servidor é capaz de criar 1 thread por cliente se conectar
//Cada uma dessas thread's vai estar enviando os dados adquiridos:
//      - em tempo real
//      - na taxa de amostragem solicitada
//      - no formato CSV
//Obs: Outros formatos já foram implementados (RAW e JSON) mas foram retirados
void TrigaServer::createServer(int port)
{
    //Variáveis para armazenar o número do socket do servidor e dos clientes
    int serverSocket, clientSocket;

    //Estruturas para armazenar endereços (IP+porta)
    struct sockaddr_in serverAddr, clientAddr;

    //Tamanho da estrutura de endereço do cliente
    socklen_t clientLen = sizeof(clientAddr);

    //Cria socket para servidor: TCP/IP e IPv4
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //Verifica se socket foi criado com sucesso
    if (serverSocket < 0) 
    {
        std::cerr << "[startServer] Error opening socket" << std::endl;
        return;
    }

    //Configuração do Endereço do Servidor
    memset(&serverAddr, 0, sizeof(serverAddr));//Zera a estrutura
    serverAddr.sin_family = AF_INET;//Família de endereços IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;//Aceitar conexões em qualquer interface de rede
    serverAddr.sin_port = htons(port);//Converte a porta para formato de rede (big-endian)

    //Vinculação (Bind) do Socket: Associa o socket ao endereço e porta especificados
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        std::cerr << "[startServer] Error on binding" << std::endl;
        return;
    }

    //Colocando socket em Modo Escuta
    listen(serverSocket, 5);//5 indica o tamanho da fila de conexões pendentes
    //std::cout << "[startServer] Server started on port " << port << std::endl;

    //Loop para enternamente aceitar novas conexões
    while(true)
    {
        //Cria um socket para o cliente que acabou de se conectar
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);//accept() bloqueia a execução do código até que um cliente se conecte

        //Se accept() por qualquer motivo falhar, reset o loop (não crie nova thread)
        if (clientSocket < 0)
        {
            std::cerr << "[startServer] Error on accept" << std::endl;
            continue;
        }

        //std::cout << "[startServer] Client connected" << std::endl;
        
        //Para cada nova conexão, cria uma thread que executa handleTCPClients, informando o socket do cliente que acabou de se conectar
        std::thread clientThread(&TrigaServer::handleTCPClients, this, clientSocket);

        //Desvincule a thread antes de executar novo loop
        clientThread.detach();
    }
}

//Metodo para lidar com o cliente:
//      - receber taxa de amostragem solicitada
//      - criar thread que envia aquisição de dados na taxa solicitada
void TrigaServer::handleTCPClients(int clientSocket)
{
    //Cria um buffer de 1024 bytes para receber dados
    char buffer[1024];

    //Bloqueia a execução até receber dados do cliente
    int n = recv(clientSocket, buffer, sizeof(buffer), 0);

    //Se houve erro na recepção, feche o socket e encerre o método
    if (n <= 0)
    {
        std::cerr << "[handleTCPClients] Error receiving data" << std::endl;
        close(clientSocket);
        return;
    }

    //Caso algum digito não numero for recebido, encerre o método
    for (int i = 0; i < n-1; ++i) if (!isdigit(buffer[i])) 
    {
        std::cerr << "[handleTCPClients] Error: client sent a not number" << std::endl;
        close(clientSocket);
        return;
    }

    //Converte os dados recebidos (string) em um número inteiro
    int interval = std::stoi(std::string(buffer, n));

    //std::cout << "[handleTCPClients] Received interval: " << interval << "ms" << std::endl;


    // Cria uma nova thread desvinculada para lidar com a comunicação contínua
    std::thread([this, interval, clientSocket]()
    {
        //Cria ponteiros compartilhados para estruturas de dados locais
        auto data_local_spuChA = std::shared_ptr <SPU_DATA> (new SPU_DATA);
        auto data_local_spuChB = std::shared_ptr <SPU_DATA> (new SPU_DATA);
        auto data_local_plc    = std::shared_ptr <PLC_DATA> (new PLC_DATA);
        ALL_DATA data;

        //Primeiramente envie o cabeçalho CSV pré-definido uma única vez
        if(send(clientSocket, CSV_HEADER, sizeof(CSV_HEADER), 0) <= 0) return;

        //Envie eternamente o conteúdo das variáveis
        while(true)
        {
            //Carregue de forma segura o conteúdo dos ponteiros
            //Objetivo:
            //      - Obter uma cópia local thread-safe dos dados globais
            //      - Permitir processamento local sem bloquear outras threads
            data_local_spuChA = data_global_spuCh[0].load();
            data_local_spuChB = data_global_spuCh[1].load();
            data_local_plc    = data_global_plc.load();

            //Salva na estrutura ALL_DATA
            //ALL_DATA era útil no contexto de enviar dados no formato RAW
            data.SPU_CHA = *data_local_spuChA;
            data.SPU_CHB = *data_local_spuChB;
            data.PLC     = *data_local_plc;
            
            //Gere a string com contéudo das variáveis no formato CSV
            std::string      buffer = genString(data);
            
            //Envie o conteudo de buffer na quantidade identificada por length()
            if(send(clientSocket, buffer.c_str(), buffer.length(), 0) <= 0) break;

            //Dorme pelo intervalo especificado
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }).detach();
}

// Ler dados da SPU usando libModbusSystematomSPU
void TrigaServer::readModbusRTU(libModbusSystematomSPU& spu)
{
    //Criação do ponteiro inteligente local
    auto data_local = std::shared_ptr <SPU_DATA> (new SPU_DATA);

    //Definindo qual SPU essa thread representa
    int adressSpu;
    if      (spu.get_portname() == adressSpuA) adressSpu = 0;
    else if (spu.get_portname() == adressSpuB) adressSpu = 1;
    else
    {
        std::cerr << "ERRO TrigaServer::readModbusRTU: erro em get_portname()";
        return;
    }

    //Loop para ler eternamente os dados da referida SPU
    while (true)
    {
        //Realizando tentativa de leitura
        *data_local  = spu.get_all();

        //Armazenar resultado no ponteiro inteligente global spuChA
        data_global_spuCh[adressSpu].store(data_local);

        //Se o valor de STATE for igual a 1 (erro de leitura) ou 2 (desconectado) pause a thread pelo determinado intervalo e tente reconectar
        if(data_local->STATE) 
        {
            std::this_thread::sleep_for(std::chrono::seconds(errorIntervalSPU));
            spu.tryConnect();
        }
        //OBS: A pausa TEM que ser depois de armazenar data_local no ponteiro global
        //Caso contrario o valor poderá ficar congelado no ponteiro global com STATE = 0.
    }
}

//Ler dados Modbus do PLC usando libModbusSiemensPLC
//Mesma lógica de readModbusRTU
void TrigaServer::readOpcTCP(libOpcTrigaPLC& plc) 
{
    auto data_local = std::shared_ptr <PLC_DATA> (new PLC_DATA);
    while (true)
    {
        *data_local = plc.get_all();
        data_global_plc.store(data_local);
        if(data_local->STATE) //Caso STATE for diferente de 0
        {
            std::this_thread::sleep_for(std::chrono::seconds(errorIntervalPLC));
            if(data_local->STATE==2) //Diferente do readModbusRTU, só tente reconectar ao PLC caso o erro seja "desconexão"
                plc.tryConnect();
        }
    }
}

//Método para decodificar time_point em vários números inteiros (estrutura int_TIME)
int_TIME TrigaServer::decodeTime(std::chrono::system_clock::time_point t)
{
    auto now = std::chrono::system_clock::to_time_t(t);
    std::tm tm_local = *std::localtime(&now);

    int_TIME time;
    time.year = tm_local.tm_year + 1900; // Ano: tm_year é anos desde 1900
    time.month = tm_local.tm_mon + 1; // Mês: tm_mon é 0-based
    time.day = tm_local.tm_mday; // Dia do mês
    time.hour = tm_local.tm_hour; // Hora
    time.minute = tm_local.tm_min; // Minuto
    time.second = tm_local.tm_sec; // Segundo

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count();
    time.millisecond = ms % 1000;
    return time;
}

//Gerar string contendo a linha do arquivo CSV com valores das variáveis
std::string TrigaServer::genString(ALL_DATA all_data) 
{
    //Decodificar tempo de leitura de cada hardware
    int_TIME SPU_CHA_TIME = decodeTime(all_data.SPU_CHA.TIME);
    int_TIME SPU_CHB_TIME = decodeTime(all_data.SPU_CHB.TIME);
    int_TIME PLC_TIME     = decodeTime(all_data.PLC.TIME);

    //Converter dados do PLC
    PLC_DATA plc_conv     = plc.convAllData(all_data.PLC);

    //Buffer para armazenar string a ser gerada
    char buffer[8192];

    //Gera a string no formato do CSV_TEMPLATE com o conteúdo de cada variável
    sprintf(buffer, CSV_TEMPLATE, all_data.SPU_CHA.STATE,
            SPU_CHA_TIME.year,
            SPU_CHA_TIME.month,
            SPU_CHA_TIME.day,
            SPU_CHA_TIME.hour,
            SPU_CHA_TIME.minute,
            SPU_CHA_TIME.second,
            SPU_CHA_TIME.millisecond,
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
            SPU_CHB_TIME.year,
            SPU_CHB_TIME.month,
            SPU_CHB_TIME.day,
            SPU_CHB_TIME.hour,
            SPU_CHB_TIME.minute,
            SPU_CHB_TIME.second,
            SPU_CHB_TIME.millisecond,
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
            PLC_TIME.year,
            PLC_TIME.month,
            PLC_TIME.day,
            PLC_TIME.hour,
            PLC_TIME.minute,
            PLC_TIME.second,
            PLC_TIME.millisecond,
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
            all_data.PLC.SVasPri,

            plc_conv.STATE,
            PLC_TIME.year,
            PLC_TIME.month,
            PLC_TIME.day,
            PLC_TIME.hour,
            PLC_TIME.minute,
            PLC_TIME.second,
            PLC_TIME.millisecond,
            plc_conv.BarraReg,
            plc_conv.BarraCon,
            plc_conv.BarraSeg,
            plc_conv.CLogALog,
            plc_conv.CLogALin,
            plc_conv.CLogAPer,
            plc_conv.CParALin,
            plc_conv.CParALog,
            plc_conv.CParAPer,
            plc_conv.CLogARea,
            plc_conv.CLin,
            plc_conv.CPer,
            plc_conv.SRadAre,
            plc_conv.SRadEntPri,
            plc_conv.SRadPoc,
            plc_conv.SRadRes,
            plc_conv.SRadSaiSec,
            plc_conv.SRadAer,
            plc_conv.SVasPri
    );
    return buffer;
}