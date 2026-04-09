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
//main.cpp
#include "TrigaServer.h"
#include <cxxopts.hpp>
#include <fstream>
#include <string>
#include <chrono>
#include <csignal>

void showVersion()
{
    std::cout << "Development Version" << std::endl;
}

void showLicense()
{
    std::cout << "TrigaServer      Copyright (C) 2023-2024      Thalles Campagnani" << std::endl;
    std::cout << "This    program    comes    with    ABSOLUTELY    NO   WARRANTY;" << std::endl;
    std::cout << "This is free software,    and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the source code." << std::endl << std::endl;
}

//Estrutura de dados contendo as variáveis de configuração do TrigaServer
//Conteúdo das variáveis pode ser alterado por parâmetros da linha de comando
struct CONFIG
{
    std::string spu_sp1             = "/dev/modbus/spuChA";       //SPU_CH_A serial port
    std::string spu_sp2             = "/dev/modbus/spuChB";       //SPU_CH_B serial port
    std::string plc_adress          = "192.168.1.1:4840";         //CLP IP:PORT
    std::string plc_conv_file       = "";                         //CLP conv file
    short       port_csv            = 1234;                       //Port of server CSV
    int         error_interval_spu  = 2;                          //Intervalo de espera para reconexão com SPU em caso de erro
    int         error_interval_plc  = 2;                          //Intervalo de espera para reconexão com PLC em caso de erro
    int         monitor_interval    = 3;                          //Intervalo de dados para monitor variáveis
    bool        monitor_stdout      = 0;                          //Exibir monitor de variáveis na saída padrão (terminal)
    bool        close               = 0;                          //Variável auxiliar: fechar programa sem iniciar servidor
};

//Função para interpretar parâmetros da linha de comando e altarar valores padrão de CONFIG
CONFIG configOptions(int argc, char* argv[])
{
    //Cria estrutura CONFIG com valores padrão
    CONFIG config;

    //Biblioteca cxxopts automatiza interpretação dos parâmetros da linha de comando
    cxxopts::Options options("TrigaServer","TrigaServer is a software for GNU operating system to get the real-time values of the Nuclear Reator Triga IPR-R1 and share in network.");
    options.add_options()
        ("v,version",          "Show the program version")
        ("h,help",             "Show this help message")
        ("l,license",          "Show info of the license")
        ("m,monitor",          "Set system monitor to STD_OUT")
        ("SPU_CHA",            "Set spuChA device file",                                        cxxopts::value<std::string>())
        ("SPU_CHB",            "Set spuChB device file",                                        cxxopts::value<std::string>())
        ("PLC_ADRESS",         "Set PLC adress in format IP:PORT",                              cxxopts::value<std::string>())
        ("PLC_CONV_FILE",      "Set convertion file for PLC values",                            cxxopts::value<std::string>())
        ("PORT_CSV",           "Set port for the server in CSV format",                         cxxopts::value<int>())
        ("INTERVAL_ERROR_SPU", "Set reconnect interval of SPU in case of error",                cxxopts::value<int>())
        ("INTERVAL_ERROR_PLC", "Set reconnect interval of PLC in case of error",                cxxopts::value<int>())
        ("INTERVAL_MONITOR",   "Set interval of system monitor",                                cxxopts::value<int>());
    auto result = options.parse(argc, argv);

    //Apenas exibir a versão e fechar o programa
    if (result.count("version") || result.count("v"))
    {
        showVersion();
        config.close = true;
        return config;
    }

    //Apenas exibir a ajuda e fechar o programa
    if (result.count("help") || result.count("h"))
    {
        std::cout << options.help() << std::endl;
        config.close = true;
        return config;
    }

    //Apenas exibir a licença e fechar o programa
    if (result.count("license") || result.count("l"))
    {
        showLicense();
        config.close = true;
        return config;
    }

    //Ativar monitor na saída padrão
    if (result.count("monitor") || result.count("m"))  config.monitor_stdout = true;
    
    //Alterar os respectivos valores, das respectivas variáveis
    if (result.count("SPU_CHA"))             config.spu_sp1            = result["SPU_CHA"].as<std::string>();
    if (result.count("SPU_CHB"))             config.spu_sp2            = result["SPU_CHB"].as<std::string>();
    if (result.count("PLC_ADRESS"))          config.plc_adress         = result["PLC_ADRESS"].as<std::string>();
    if (result.count("PLC_CONV_FILE"))       config.plc_conv_file      = result["PLC_CONV_FILE"].as<std::string>();
    if (result.count("PORT_CSV"))            config.port_csv           = result["PORT_CSV"].as<int>();
    if (result.count("INTERVAL_ERROR_SPU"))  config.error_interval_spu = result["INTERVAL_ERROR_SPU"].as<int>();
    if (result.count("INTERVAL_ERROR_PLC"))  config.error_interval_plc = result["INTERVAL_ERROR_PLC"].as<int>();
    if (result.count("INTERVAL_MONITOR"))    config.monitor_interval   = result["INTERVAL_MONITOR"].as<int>();

    return config;
}

int main(int argc, char* argv[])
{
    //Solução do problema aleatório de o programa fechar abruptamete quando algum cliente se desconecta: Ignorar sinal SIGPIPE
    //Explicação: Este problema acontece em sistemas POSIX, pois quando o cliente se desconecta, as vezes pode coincidir do programa estar escrevendo no socket via função send(). Mas quando o cliente se desconecta, o socket é fechado abruptamente. Nessa situação, o kernel linux manda o sinal SIGPIPE para o programa e o mesmo é encerrado imediatamente, antes mesmo de send() retornar erro
    signal(SIGPIPE, SIG_IGN);
    //Soluções alternativas, específicas de cada kernel:
    //[Kernel Linux]: Inibir a geração de SIGPIPE apenas para aquela chamada específica de send() com a flag MSG_NOSIGNAL
    //Ex.:    send(..., MSG_NOSIGNAL)
    //[Kernel dos BSD's (MacOS incluso)]: Desativar a geração de SIGPIPE para aquele socket específico com a flag SO_NOSIGPIPE
    //Ex.:    setsockopt(..., SO_NOSIGPIPE,...);
    //        send(...);
    //[Kernel Hurd]: Ainda não há implementado algo como a flag MSG_NOSIGNAL ou SO_NOSIGPIPE
    //[Kernel Windows NT]: não segue o modelo de sinais do Unix (POSIX), logo não existe o sinal SIGPIPE, portanto o programa não é encerrado e a função send() consegue retornar o código de erro.
    //Portanto signal(SIGPIPE, SIG_IGN) é uma solução portável entre diferente kernel's.


    //Cria objeto do tipo CONFIG com valores modificados (ou não) pela linha de comando
    CONFIG config = configOptions(argc, argv);

    //Encerre o programa caso seja passado na linha de comando parâmetros como help
    if (config.close) return 0;

    //Cria o objeto do tipo TrigaServer passando as configurações setadas
    //Ao criar esse objeto, automaticamente ele inicia a comunicação com o hardware externo e inicia a aquisição de dados
    TrigaServer server(config.spu_sp1,
                       config.spu_sp2,
                       config.plc_adress,
                       config.plc_conv_file,
                       config.error_interval_spu,
                       config.error_interval_plc);

    //Cria thread do servidor que recebe novas conexões
    //Essa thread é capaz de criar 1 thread por cliente se conectar
    //Cada uma dessas thread's vai estar enviando os dados adquiridos:
    //      - em tempo real
    //      - na taxa de amostragem solicitada
    //      - no formato CSV
    //Obs: Outros formatos já foram implementados (RAW e JSON) mas foram retirados
    std::thread serverCsvThread (&TrigaServer::createServer, &server, config.port_csv);
    
    //Desvincule a thread criada da thread que a criou (essa)
    //Sem isso teoricamente causaria erro std::terminate caso main chegasse na linha "return 0;"
    //Mas na prática ela nunca chega ao fim, na implementação atual
    serverCsvThread.detach();

    //Espere 3 segundo antes de abrir o monitor
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //Monitor de sistema
    //Cria um arquivo de log temporário com a situação do servidor
    //Caso seja passado como parâmetro, também emite o log na saída padrão
    std::ofstream outputFile("/tmp/trigaserver.systemmonitor");
    while (true)
    {
        //Vetor contendo o estado de cada hardware
        std::vector<int> state = server.state();
        
        //Vetor contendo o estado de potência e período do reator, lido a partir de cada hardware
        std::vector<float> PP = server.readPP();
        
        //String contendo a saída de log formatada
        //Obs: falta adicionar data-hora
        std::string screen = "TrigaServer - System Monitor\n";
                    screen += "\nClients Side\n";
                    if(config.port_csv)  screen += "Server  CSV:   PORT=" + std::to_string(config.port_csv)           + "\n";
                    //screen += "Num. Clients:?\n"; //Numero de clientes ainda não monitorado
                    screen += "\nMachine Side\n";
                    screen += "SPU_A:    STATE="     + std::to_string(state[0]) + "\t\tPORT=" + config.spu_sp1        + "\n";
                    screen += "SPU_B:    STATE="     + std::to_string(state[1]) + "\t\tPORT=" + config.spu_sp2        + "\n";
                    screen += "PLC:      STATE="     + std::to_string(state[2]) + "\t\tPORT=" + config.plc_adress     + "\n";
                    screen += "\nImportante Values\n";
                    screen += "SPU_A:    N="         + std::to_string(PP[0])    + "\t\tT="    + std::to_string(PP[1]) + "\n";
                    screen += "SPU_B:    N="         + std::to_string(PP[2])    + "\t\tT="    + std::to_string(PP[3]) + "\n";
                    screen += "PLC:      N="         + std::to_string(PP[4])    + "\t\tT="    + std::to_string(PP[6]) + "\n";
                    screen += "      N_log="         + std::to_string(PP[5])                                          + "\n";
                    screen += "\n";
        
        //Escreva no arquivo
        outputFile << screen;

        //Se estiver definido para emitir o log também na saída padrão:
        //      -Limpe a saída padrão
        //      -Escreva na saída padrão
        //Dessa forma fica legível para debugar a informação pelo terminal
        if(config.monitor_stdout)
        {
            system("clear");
            std::cout  << screen;
        }

        //Espere o tempo definido antes de emitir novo log
        std::this_thread::sleep_for(std::chrono::seconds(config.monitor_interval));

        //Não foi implementado nenhuma condição de parada:
        //break;
    }

    //Atualmente nunca executado
    outputFile.close();
    return 0;
}
