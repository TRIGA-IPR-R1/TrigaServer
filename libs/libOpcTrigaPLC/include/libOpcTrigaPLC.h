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

#pragma once

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <vector>

struct PLC_DATA
{
  int STATE = -1; // Status do PLC:
                  // -1 = Nenhuma tentativa de conexão realizada ainda.
                  //  0 = Valores lidos com sucesso.
                  //  1 = Erro de leitura
                  //  2 = Servidor desconectado
  std::chrono::system_clock::time_point TIME;
  // 
  float BarraReg      = -123456;      //ns=2;s=IoConfig_Globals_Mapping.inBarraReg (%IW5)   //Barra de Regulação
  float BarraCon      = -123456;      //ns=2;s=IoConfig_Globals_Mapping.inBarraCon (%IW6)   //Barra de Controle
  float BarraSeg      = -123456;      //ns=2;s=IoConfig_Globals_Mapping.inBarraSeg (%IW7)   //Barra de Segurança
  int   CLinScale     = -123456;      //ns=2;IoConfig_Globals_Mapping.inModulo_D0  (%IW0)   //Canal Linear - Escala (Bit 0, 1 e 3)
  float CLin          = -123456;      //ns=2;IoConfig_Globals_Mapping.inCLin (%IW8)         //Canal Linear
  float CPer          = -123456;      //ns=2;IoConfig_Globals_Mapping.inCPer (%IW13)        //Canal Percentual
  float CLogARea      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCLogARea (%IW14)    //Canal Logarítimo - Aquisição Reativímetro
  float CLogALin      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCLogALin (%IW15)    //Canal Logaritico - Aquisição Linear
  float CLogALog      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCLogALog (%IW16)    //Canal Logaritico - Aquisição Logarítima
  float CLogAPer      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCLogAPer (%IW17)    //Canal Logarítimo - Aquisição Período
  float CParALin      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCParALin (%IW18)    //Canal Partida - Aquisição Linear
  float CParALog      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCParALog (%IW19)    //Canal Partida - Aquisição Log
  float CParAPer      = -123456;      //ns=2;IoConfig_Globals_Mapping.inCParAPer (%IW20)    //Canal Partida - Aquisição Período
  float SRadAre       = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadAre (%IW25)     //Sensor Radiação Área
  float SRadEntPri    = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadEntPri (%IW26)  //Sensor Radiação Entrada Primário
  float SRadPoc       = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadPoc (%IW27)     //Sensor Radiação Poço
  float SRadRes       = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadRes (%IW28)     //Sensor Radiação Resinas
  float SRadSaiSec    = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadSaiSec (%IW29)  //Sensor Radiação Saída Secundário
  float SRadAer       = -123456;      //ns=2;IoConfig_Globals_Mapping.inSRadAer (%IW30)     //Sensor Radiação Aerossois
  float SVasPri       = -123456;      //ns=2;IoConfig_Globals_Mapping.inSVasPri (%IW49)     //Sensor Vazão Sistema Primário de Refrigeração
};

// Matriz Nx2 para os dados do PLC
// Coluna 0 = Header
// Coluna 1 = Tipo
inline std::vector<std::vector<std::string>> PLC_DATA_MATRIX = {
    {"PLC_%s_STATE",      "%d"},
    {"PLC_%s_TIME_Y",     "%d"},
    {"PLC_%s_TIME_Mo",    "%d"},
    {"PLC_%s_TIME_D",     "%d"},
    {"PLC_%s_TIME_H",     "%d"},
    {"PLC_%s_TIME_Mi",    "%d"},
    {"PLC_%s_TIME_S",     "%d"},
    {"PLC_%s_TIME_MS",    "%d"},
    {"PLC_%s_BarraReg",   "%f"},
    {"PLC_%s_BarraCon",   "%f"},
    {"PLC_%s_BarraSeg",   "%f"},
    {"PLC_%s_CLogALog",   "%f"},
    {"PLC_%s_CLogALin",   "%f"},
    {"PLC_%s_CLogAPer",   "%f"},
    {"PLC_%s_CParALin",   "%f"},
    {"PLC_%s_CParALog",   "%f"},
    {"PLC_%s_CParAPer",   "%f"},
    {"PLC_%s_CLogARea",   "%f"},
    {"PLC_%s_CLin",       "%f"},
    {"PLC_%s_CPer",       "%f"},
    {"PLC_%s_SRadAre",    "%f"},
    {"PLC_%s_SRadEntPri", "%f"},
    {"PLC_%s_SRadPoc",    "%f"},
    {"PLC_%s_SRadRes",    "%f"},
    {"PLC_%s_SRadSaiSec", "%f"},
    {"PLC_%s_SRadAer",    "%f"},
    {"PLC_%s_SVasPri",    "%f"}
};

struct CONV_LIN
{
  float x0 =  1;
  float y0 =  1;
  float x1 =  0;
  float y1 = -2;
  std::string unit;
};

struct CONV_LOG
{
  double a = -2;
  double b =  0;
  std::string unit;
};

struct CONV_PER
{
  double a = 5000;  //Tensão referente +/-inf
  double b = 0;     //Multiplicação
  double c = 0;     //Não usado ainda
  double d = 0;     //Não usado ainda
  std::string unit;
};

struct CONV_VAZ //Dúvidas: https://github.com/TRIGA-IPR-R1/Vazao-Placa-Orificio
{
  double a = 18.753;  //'x' e 'y':Conversão de mA para deltaP
  double b =-74.956;
  double c = 2.417;   //'z': Conversão de deltaP para vazão
  std::string unit;
};

struct CONV_PLC {
  CONV_LIN BarraReg{.unit = ""};
  CONV_LIN BarraCon{.unit = ""};
  CONV_LIN BarraSeg{.unit = ""};
  CONV_LIN CLogALin{.unit = "W"};
  CONV_LOG CLogALog{.unit = "W"};
  CONV_PER CLogAPer{.unit = "s"};
  CONV_LIN CParALin{.unit = "W"};
  CONV_LOG CParALog{.unit = "W"};
  CONV_PER CParAPer{.unit = "s"};
  CONV_LIN CLogARea{.unit = "pcm"};
  CONV_LIN CLin{.unit = "W"};
  CONV_LIN CPer{.unit = "%"};
  CONV_LOG SRadAre{.unit = "mRem/h"};
  CONV_LOG SRadEntPri{.unit = "mRem/h"};
  CONV_LOG SRadPoc{.unit = "mRem/h"};
  CONV_LOG SRadRes{.unit = "mRem/h"};
  CONV_LOG SRadSaiSec{.unit = "mRem/h"};
  CONV_LOG SRadAer{.unit = "mRem/h"};
  CONV_VAZ SVasPri{.unit = "m3/h"};
};

inline std::string convUnit(const CONV_PLC& conv, const std::string& field)
{
  if (field == "BarraReg")   return conv.BarraReg.unit;
  if (field == "BarraCon")   return conv.BarraCon.unit;
  if (field == "BarraSeg")   return conv.BarraSeg.unit;
  if (field == "CLogALin")   return conv.CLogALin.unit;
  if (field == "CLogALog")   return conv.CLogALog.unit;
  if (field == "CLogAPer")   return conv.CLogAPer.unit;
  if (field == "CParALin")   return conv.CParALin.unit;
  if (field == "CParALog")   return conv.CParALog.unit;
  if (field == "CParAPer")   return conv.CParAPer.unit;
  if (field == "CLogARea")   return conv.CLogARea.unit;
  if (field == "CLin")       return conv.CLin.unit;
  if (field == "CPer")       return conv.CPer.unit;
  if (field == "SRadAre")    return conv.SRadAre.unit;
  if (field == "SRadEntPri") return conv.SRadEntPri.unit;
  if (field == "SRadPoc")    return conv.SRadPoc.unit;
  if (field == "SRadRes")    return conv.SRadRes.unit;
  if (field == "SRadSaiSec") return conv.SRadSaiSec.unit;
  if (field == "SRadAer")    return conv.SRadAer.unit;
  if (field == "SVasPri")    return conv.SVasPri.unit;
  return "";
}

inline void setConvUnit(CONV_PLC& conv, const std::string& field, const std::string& unit)
{
  if (field == "BarraReg")        conv.BarraReg.unit = unit;
  else if (field == "BarraCon")   conv.BarraCon.unit = unit;
  else if (field == "BarraSeg")   conv.BarraSeg.unit = unit;
  else if (field == "CLogALin")   conv.CLogALin.unit = unit;
  else if (field == "CLogALog")   conv.CLogALog.unit = unit;
  else if (field == "CLogAPer")   conv.CLogAPer.unit = unit;
  else if (field == "CParALin")   conv.CParALin.unit = unit;
  else if (field == "CParALog")   conv.CParALog.unit = unit;
  else if (field == "CParAPer")   conv.CParAPer.unit = unit;
  else if (field == "CLogARea")   conv.CLogARea.unit = unit;
  else if (field == "CLin")       conv.CLin.unit = unit;
  else if (field == "CPer")       conv.CPer.unit = unit;
  else if (field == "SRadAre")    conv.SRadAre.unit = unit;
  else if (field == "SRadEntPri") conv.SRadEntPri.unit = unit;
  else if (field == "SRadPoc")    conv.SRadPoc.unit = unit;
  else if (field == "SRadRes")    conv.SRadRes.unit = unit;
  else if (field == "SRadSaiSec") conv.SRadSaiSec.unit = unit;
  else if (field == "SRadAer")    conv.SRadAer.unit = unit;
  else if (field == "SVasPri")    conv.SVasPri.unit = unit;
}

void libOpcTrigaPLC_license();

struct libOpcTrigaPLC_private;

class libOpcTrigaPLC {
public:
  libOpcTrigaPLC(std::string address);
  libOpcTrigaPLC(std::string address, std::string filename);
  ~libOpcTrigaPLC();

  CONV_PLC fatorConv;
  CONV_PLC readFatorConvFile(std::string filename);

  PLC_DATA convAllData(PLC_DATA plcOrig);
  PLC_DATA get_all_conv();
  PLC_DATA get_all();
  bool tryConnect();

private:
  libOpcTrigaPLC_private *_p;

  std::string stdErrorMsg(std::string functionName, std::string errorMsg,
                          std::string exptionMsg);

  float convLin(float  x, CONV_LIN conv);
  float convLog(double x, CONV_LOG conv);
  float convPer(double x, CONV_PER conv);
  float convVaz(double x, CONV_VAZ conv);
};
