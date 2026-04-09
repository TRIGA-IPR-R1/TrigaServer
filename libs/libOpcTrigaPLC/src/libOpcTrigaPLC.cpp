/*
libOpcTrigaPLC is a library to communicate with the Triga PLC
using OPC UA client (Ethernet) on a GNU operating system.
Copyright (C) 2024-2026 Thalles Campagnani

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

#include <libOpcTrigaPLC.h>
#include <cmath>
#include <open62541pp/open62541pp.h>
#include <fstream>
#include <limits>

struct libOpcTrigaPLC_private {
    opcua::Client client;
    PLC_DATA plcData;
    std::string serverAddress;
};

void libOpcTrigaPLC_license()
{
    std::cout << "libOpcTrigaPLC    Copyright (C) 2024 Thalles Campagnani" << std::endl;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY;" << std::endl;
    std::cout << "This is free software, and you are welcome to redistribute it" << std::endl;
    std::cout << "under certain conditions; For more details read the file LICENSE" << std::endl;
    std::cout << "that came together with the library." << std::endl << std::endl;
}

bool libOpcTrigaPLC::tryConnect()
{
    try
    {
        this->_p->client.connect(this->_p->serverAddress);
    }
    catch (const std::exception& e)
    {
        std::cerr << stdErrorMsg("tryConnect()", "Erro ao tentar conectar", e.what());
        return 1;
    }
    return 0;
}

std::string libOpcTrigaPLC::stdErrorMsg(std::string functionName, std::string errorMsg, std::string exptionMsg)
{
    std::string msg =   "ERROR in libOpcTrigaPLC::" + 
                        functionName + 
                        "\n\tError type: " + 
                        errorMsg;
    if (exptionMsg!="") 
    {
        msg +=          "\n\tError code: " + 
                        exptionMsg;
    }
    msg += "\n";
    return msg;
}

libOpcTrigaPLC::libOpcTrigaPLC(std::string address)
{
    this->_p = new libOpcTrigaPLC_private;
    this->_p->serverAddress = "opc.tcp://" + address;
    tryConnect();
}

libOpcTrigaPLC::libOpcTrigaPLC(std::string address, std::string filename)
{
    this->_p = new libOpcTrigaPLC_private;
    this->_p->serverAddress = "opc.tcp://" + address;
    tryConnect();
    this->fatorConv = this->readFatorConvFile(filename);
}

libOpcTrigaPLC::~libOpcTrigaPLC()
{
    this->_p->client.disconnect();
    delete this->_p;
}

//Função para converter o valor de tensão através de 2 pontos conhecidos. 
float libOpcTrigaPLC::convLin(float tensao, CONV_LIN conv)
{
    if (tensao==-123456) return tensao;
    float deltaX = conv.x1 - conv.x0;
    if (deltaX == 0) return std::numeric_limits<float>::infinity(); // Proteção extra para erro no arquivo de configuraçõa
    
    return (tensao * (conv.y1 - conv.y0) + (conv.y0 * conv.x1 - conv.y1 * conv.x0)) / deltaX;
}

//Função para converter o valor de tensão, sendo a tesão representando um sinal logarítimo de base 10. 
float libOpcTrigaPLC::convLog(double tensao, CONV_LOG conv)
{
    if (tensao==-123456) return tensao;
    return conv.b * std::pow(10, conv.a * tensao);
}

//Função para converter o valor de x, sendo x sinal de período.
//  Lembrete: tensão varia de -10.000 a 10.000 mV
float libOpcTrigaPLC::convPer(double tensao, CONV_PER conv)
{
    if (tensao==-123456) return tensao; // Não existe valor de tensão menor que -10.000mV. Valor de inicialização da variável: -123456. Portanto nenhum valor foi lido.
    double dom = tensao-conv.a;
    if (abs(dom) < 1e-9) return std::numeric_limits<double>::infinity(); //Se o denominador for igual a 0 (menor que "epsilon"), retorne infinito
    return conv.b/dom;
}

//Função para converter o valor de corrente em vazão
//  Lembrete: corrente em microamperes: 0~20.000uA (mas a saída do sensor é 4~20mA)
float libOpcTrigaPLC::convVaz(double corrente, CONV_VAZ conv)
{
    //Verifica erro
    if (corrente==-123456) return corrente;//Se for igual a -123456, nenhum valor foi lido

    //Verifica erro
    if (corrente<3.5) return -3;//Se a corrente for menor que 3.5mA provavelmente é um erro de cabo desconectado, ou equivalente...
    
    //Converte para pressão
    double pressao = corrente*conv.a + conv.b;

    //Evita erro: cálculo de raiz negativa
    if (pressao<=0) return 0;

    //Converte para vazão
    return conv.c * sqrt(pressao);
}

//Função para converter os dados brutos do PLC
PLC_DATA libOpcTrigaPLC::convAllData(PLC_DATA plcOrig)
{
    PLC_DATA plcConv   = plcOrig;
    plcConv.BarraReg   = convLin(plcOrig.BarraReg,   fatorConv.BarraReg);//Converter bits para "posições de barra"
    plcConv.BarraCon   = convLin(plcOrig.BarraCon,   fatorConv.BarraCon);
    plcConv.BarraSeg   = convLin(plcOrig.BarraSeg,   fatorConv.BarraSeg);
    plcConv.CLogALin   = convLin(plcOrig.CLogALin,   fatorConv.CLogALin);//Converter bits para W
    plcConv.CLogALog   = convLog(plcOrig.CLogALog,   fatorConv.CLogALog);
    plcConv.CLogAPer   = convPer(plcOrig.CLogAPer,   fatorConv.CLogAPer);
    plcConv.CParALin   = convLin(plcOrig.CParALin,   fatorConv.CParALin);
    plcConv.CParALog   = convLog(plcOrig.CParALog,   fatorConv.CParALog);
    plcConv.CParAPer   = convPer(plcOrig.CParAPer,   fatorConv.CParAPer);
    plcConv.CLogARea   = convLin(plcOrig.CLogARea,   fatorConv.CLogARea);
    plcConv.CLin       = convLin(plcOrig.CLin,       fatorConv.CLin);
    plcConv.CPer       = convLin(plcOrig.CPer,       fatorConv.CPer);
    plcConv.SRadAre    = convLog(plcOrig.SRadAre,    fatorConv.SRadAer);
    plcConv.SRadEntPri = convLog(plcOrig.SRadEntPri, fatorConv.SRadEntPri);
    plcConv.SRadPoc    = convLog(plcOrig.SRadPoc,    fatorConv.SRadPoc);
    plcConv.SRadRes    = convLog(plcOrig.SRadRes,    fatorConv.SRadRes);
    plcConv.SRadSaiSec = convLog(plcOrig.SRadSaiSec, fatorConv.SRadSaiSec);
    plcConv.SRadAer    = convLog(plcOrig.SRadAer,    fatorConv.SRadAer);
    plcConv.SVasPri    = convVaz(plcOrig.SVasPri,    fatorConv.SVasPri);//Converter bits para m^3/h
    return plcConv;
}

PLC_DATA libOpcTrigaPLC::get_all_conv()
{
    this->_p->plcData = convAllData(get_all());
    return this->_p->plcData;
}

PLC_DATA libOpcTrigaPLC::get_all()
{
    try
    {
        this->_p->plcData.BarraReg     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inBarraReg (%IW5)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.BarraCon     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inBarraCon (%IW6)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.BarraSeg     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inBarraSeg (%IW7)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.CLinScale    = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inModule_D0 (%IW0)"}    )  .readValueScalar<u_int16_t>() & 0b00000111 - 2;
        this->_p->plcData.CLin         = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCLin (%IW8)"}         )  .readValueScalar<int16_t>();
        this->_p->plcData.CPer         = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCPer (%IW13)"}        )  .readValueScalar<int16_t>();
        this->_p->plcData.CLogARea     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCLogARea (%IW14)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CLogALin     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCLogALin (%IW15)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CLogALog     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCLogALog (%IW16)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CLogAPer     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCLogAPer (%IW17)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CParALin     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCParALin (%IW18)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CParALog     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCParALog (%IW19)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.CParAPer     = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inCParAPer (%IW20)"}    )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadAre      = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadAre (%IW25)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadEntPri   = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadEntPri (%IW26)"}  )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadPoc      = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadPoc (%IW27)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadRes      = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadRes (%IW28)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadSaiSec   = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadSaiSec (%IW29)"}  )  .readValueScalar<int16_t>();
        this->_p->plcData.SRadAer      = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSRadAer (%IW30)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.SVasPri      = this->_p->client.getNode( {2, "IoConfig_Globals_Mapping.inSVasPri (%IW49)"}     )  .readValueScalar<int16_t>();
        this->_p->plcData.STATE = 0;
    }
    catch (const std::exception& e)
    {
        if (this->_p->client.isConnected())
        {
            std::cerr << stdErrorMsg("get_all()", "Cliente conectado, porém erro ao adquirir dados", e.what());
            this->_p->plcData.STATE = 1;
        }
        else
        {
            std::cerr << stdErrorMsg("get_all()", "Cliente desconectado", e.what());
            this->_p->plcData.STATE = 2;
        }
    }

    this->_p->plcData.TIME = std::chrono::system_clock::now();
    return this->_p->plcData;
}

//CONV_LIN 

CONV_PLC libOpcTrigaPLC::readFatorConvFile(std::string filename)
{
    CONV_PLC fatorConv;
    if (filename=="") return fatorConv;

    std::ifstream infile(filename);
    std::string line;

    std::string kind;

    //cout << "before while\n";
    while (std::getline(infile, line)) 
    {
        // Remove espaços em branco no início e no fim da linha
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty() || line[0] == '#')
        {
            continue;  // Pula linhas vazias e comentários
        }
        if (line[0] == '[' && line.back() == ']') 
        {
            std::string section = line.substr(1, line.size() - 2);
            std::istringstream iss(section);
            iss >> kind;
        } 
        else 
        {
            std::istringstream iss(line);
            std::string key;
            std::string igual, valueS;
            iss >> key >> igual >> valueS;
            double value = std::stod(valueS);

            if (kind == "BarraReg") 
            {
                if      (key == "x0") fatorConv.BarraReg.x0 = value;
                else if (key == "x1") fatorConv.BarraReg.x1 = value;
                else if (key == "y0") fatorConv.BarraReg.y0 = value;
                else if (key == "y1") fatorConv.BarraReg.y1 = value;
            } 
            else if (kind == "BarraCon") 
            {
                if      (key == "x0") fatorConv.BarraCon.x0 = value;
                else if (key == "x1") fatorConv.BarraCon.x1 = value;
                else if (key == "y0") fatorConv.BarraCon.y0 = value;
                else if (key == "y1") fatorConv.BarraCon.y1 = value;
            } 
            else if (kind == "BarraSeg") 
            {
                if      (key == "x0") fatorConv.BarraSeg.x0 = value;
                else if (key == "x1") fatorConv.BarraSeg.x1 = value;
                else if (key == "y0") fatorConv.BarraSeg.y0 = value;
                else if (key == "y1") fatorConv.BarraSeg.y1 = value;
            } 
            else if (kind == "CLogALin") 
            {
                if      (key == "x0") fatorConv.CLogALin.x0 = value;
                else if (key == "x1") fatorConv.CLogALin.x1 = value;
                else if (key == "y0") fatorConv.CLogALin.y0 = value;
                else if (key == "y1") fatorConv.CLogALin.y1 = value;
            } 
            else if (kind == "CParALin") 
            {
                if      (key == "x0") fatorConv.CParALin.x0 = value;
                else if (key == "x1") fatorConv.CParALin.x1 = value;
                else if (key == "y0") fatorConv.CParALin.y0 = value;
                else if (key == "y1") fatorConv.CParALin.y1 = value;
            } 
            else if (kind == "CLogARea") 
            {
                if      (key == "x0") fatorConv.CLogARea.x0 = value;
                else if (key == "x1") fatorConv.CLogARea.x1 = value;
                else if (key == "y0") fatorConv.CLogARea.y0 = value;
                else if (key == "y1") fatorConv.CLogARea.y1 = value;
            } 
            else if (kind == "CLin") 
            {
                if      (key == "x0") fatorConv.CLin.x0 = value;
                else if (key == "x1") fatorConv.CLin.x1 = value;
                else if (key == "y0") fatorConv.CLin.y0 = value;
                else if (key == "y1") fatorConv.CLin.y1 = value;
            } 
            else if (kind == "CPer") 
            {
                if      (key == "x0") fatorConv.CPer.x0 = value;
                else if (key == "x1") fatorConv.CPer.x1 = value;
                else if (key == "y0") fatorConv.CPer.y0 = value;
                else if (key == "y1") fatorConv.CPer.y1 = value;
            } 
            else if (kind == "SVasPri") 
            {
                if      (key == "a") fatorConv.SVasPri.a = value;
                else if (key == "b") fatorConv.SVasPri.b = value;
                else if (key == "c") fatorConv.SVasPri.c = value;
            } 
            else if (kind == "CLogALog") 
            {
                if      (key == "a") fatorConv.CLogALog.a = value;
                else if (key == "b") fatorConv.CLogALog.b = value;
            } 
            else if (kind == "CParALog") 
            {
                if      (key == "a") fatorConv.CParALog.a = value;
                else if (key == "b") fatorConv.CParALog.b = value;
            } 
            else if (kind == "SRadAre") 
            {
                if      (key == "a") fatorConv.SRadAre.a = value;
                else if (key == "b") fatorConv.SRadAre.b = value;
            } 
            else if (kind == "SRadEntPri") 
            {
                if      (key == "a") fatorConv.SRadEntPri.a = value;
                else if (key == "b") fatorConv.SRadEntPri.b = value;
            } 
            else if (kind == "SRadPoc") 
            {
                if      (key == "a") fatorConv.SRadPoc.a = value;
                else if (key == "b") fatorConv.SRadPoc.b = value;
            } 
            else if (kind == "SRadRes") 
            {
                if      (key == "a") fatorConv.SRadRes.a = value;
                else if (key == "b") fatorConv.SRadRes.b = value;
            } 
            else if (kind == "SRadSaiSec") 
            {
                if      (key == "a") fatorConv.SRadSaiSec.a = value;
                else if (key == "b") fatorConv.SRadSaiSec.b = value;
            } 
            else if (kind == "SRadAer") 
            {
                if      (key == "a") fatorConv.SRadAer.a = value;
                else if (key == "b") fatorConv.SRadAer.b = value;
            } 
            else if (kind == "CLogAPer") 
            {
                if      (key == "a") fatorConv.CLogAPer.a = value;
                else if (key == "b") fatorConv.CLogAPer.b = value;
                else if (key == "c") fatorConv.CLogAPer.c = value;
                else if (key == "d") fatorConv.CLogAPer.d = value;
            } 
            else if (kind == "CParAPer") 
            {
                if      (key == "a") fatorConv.CParAPer.a = value;
                else if (key == "b") fatorConv.CParAPer.b = value;
                else if (key == "c") fatorConv.CParAPer.c = value;
                else if (key == "d") fatorConv.CParAPer.d = value;
            }
        }
    }

    return fatorConv;
}
