Bibliotecas internas
====================

O executável liga estaticamente duas bibliotecas que vivem em
``libs/``. Elas também existem como repositórios próprios.

libModbusSystematomSPU
----------------------

Comunicação com a SPU SystemAtom via **Modbus-RTU** (RS-485), usando
libmodbus.

Classe pública: ``libModbusSystematomSPU``.

Construtor
~~~~~~~~~~

.. code-block:: cpp

   libModbusSystematomSPU(std::string portname);

Métodos principais
~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Método
     - Descrição
   * - ``tryConnect()``
     - Tenta (re)abrir o contexto Modbus na porta serial.
   * - ``get_portname()``
     - Devolve o caminho da porta (usado pelo TrigaServer para
       distinguir ChA e ChB).
   * - ``get_all()``
     - Lê o conjunto completo de registradores e devolve ``SPU_DATA``.
   * - ``get_all_update_NT()``
     - Atualiza só potência e período.
   * - ``get_all_update_F()``
     - Atualiza as grandezas ``F1``–``F3``.
   * - ``get_all_update_NTF()``
     - Atualiza N, T e F.
   * - ``get_all_update_bool()``
     - Atualiza os flags digitais.
   * - ``get_N_DATA_FP()`` etc.
     - Getters individuais.

O TrigaServer usa apenas ``get_all()`` e ``tryConnect()``.

Estrutura ``SPU_DATA``
~~~~~~~~~~~~~~~~~~~~~~

Campos documentados em :doc:`protocolo`. Valores iniciais são ``-1``
(inteiros/floats) e ``STATE = -1``.

libOpcTrigaPLC
--------------

Cliente **OPC UA** da mesa de controle, baseado em open62541pp
(baixado no configure via FetchContent).

Classe pública: ``libOpcTrigaPLC``.

Construtores
~~~~~~~~~~~~

.. code-block:: cpp

   libOpcTrigaPLC(std::string address);
   libOpcTrigaPLC(std::string address, std::string filename);

O segundo lê o arquivo de conversão (veja :doc:`configuracao`).

Métodos principais
~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Método
     - Descrição
   * - ``get_all()``
     - Lê os nós OPC UA e devolve ``PLC_DATA`` bruto.
   * - ``get_all_conv()``
     - Lê e já aplica ``convAllData()``.
   * - ``convAllData(PLC_DATA)``
     - Aplica os fatores de ``fatorConv`` (linear, log, período,
       vazão).
   * - ``tryConnect()``
     - Reconecta ao servidor OPC UA.
   * - ``readFatorConvFile(filename)``
     - Recarrega os fatores a partir do INI.

O TrigaServer lê com ``get_all()`` e converte na hora de montar o CSV
(``convAllData``), para publicar bruto e convertido na mesma linha.

Estrutura ``PLC_DATA``
~~~~~~~~~~~~~~~~~~~~~~

Campos documentados em :doc:`protocolo`. O fallback numérico de
grandezas não lidas é ``-123456``.

FormatCSV
---------

Namespace local em ``include/libFormatCSV.h`` /
``src/libFormatCSV.cpp``. Não é uma biblioteca instalável.

.. code-block:: cpp

   namespace FormatCSV {
       std::string generateCSVHeader();
       std::string generateCSVTemplate();
   }

As duas funções percorrem ``PLC_DATA_MATRIX`` (duas vezes: ``CONV`` e
``ORIG``) e ``SPU_DATA_MATRIX`` (``ChA`` e ``ChB``), na mesma ordem
usada por ``TrigaServer::genString()``.
