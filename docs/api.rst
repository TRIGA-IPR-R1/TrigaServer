Referência da API
=================

Classe ``TrigaServer``
----------------------

Declarada em ``include/TrigaServer.h``.

Construtor
~~~~~~~~~~

.. code-block:: cpp

   TrigaServer(std::string spu_sp1,
               std::string spu_sp2,
               std::string clp_adress,
               std::string clp_conv_file,
               int error_interval_plc,
               int error_interval_spu);

Cria os objetos ``libModbusSystematomSPU`` (duas portas) e
``libOpcTrigaPLC`` (endereço + arquivo de conversão) e chama
``startReadThreads()``.

.. warning::

   No construtor, ``error_interval_plc`` é gravado em
   ``errorIntervalSPU`` e ``error_interval_spu`` em
   ``errorIntervalPLC``. Os nomes dos parâmetros e dos membros estão
   cruzados em relação à ordem dos argumentos. A linha de comando em
   ``main.cpp`` passa SPU e depois PLC, portanto os intervalos
   efetivos ficam invertidos em relação aos nomes. Vale conferir o
   código se for alterar essa parte.

Destrutor
~~~~~~~~~

.. code-block:: cpp

   ~TrigaServer();

Vazio. As conexões não são encerradas de forma explícita; o processo
é normalmente morto pelo systemd.

Métodos públicos
~~~~~~~~~~~~~~~~

``void createServer(int port)``
   Cria o socket TCP, faz bind na porta, escuta e, em loop, aceita
   clientes. Cada cliente vira uma thread ``handleTCPClients``.

``std::vector<int> state()``
   Devolve ``{STATE_SPU_A, STATE_SPU_B, STATE_PLC}``. Usado pelo
   monitor.

``std::vector<float> readPP()``
   Devolve potência e período de cada hardware, nesta ordem:

   0. SPU-A ``N_DATA_FP``
   1. SPU-A ``T_DATA_FP``
   2. SPU-B ``N_DATA_FP``
   3. SPU-B ``T_DATA_FP``
   4. CLP ``CLogALin``
   5. CLP ``CLogALog``
   6. CLP ``CLogAPer``

Métodos privados
~~~~~~~~~~~~~~~~

``void startReadThreads()``
   Lança e destaca as três threads de aquisição.

``void readModbusRTU(libModbusSystematomSPU& spu)``
   Loop de leitura de uma SPU. Identifica o canal por
   ``get_portname()``.

``void readOpcTCP(libOpcTrigaPLC& plc)``
   Loop de leitura do CLP.

``void handleTCPClients(int clientSocket)``
   Recebe o intervalo e lança a thread que envia CSV.

``std::string genString(ALL_DATA all_data)``
   Monta uma linha CSV a partir de ``ALL_DATA``.

``int_TIME decodeTime(std::chrono::system_clock::time_point t)``
   Quebra um ``time_point`` em ano, mês, dia, hora, minuto, segundo e
   milissegundo (hora local).

Estruturas auxiliares
---------------------

ALL_DATA
~~~~~~~~

.. code-block:: cpp

   struct ALL_DATA {
       PLC_DATA PLC;
       SPU_DATA SPU_CHA;
       SPU_DATA SPU_CHB;
   };

Agrega uma amostra completa dos três hardwares. Restou da época em que
havia saída RAW.

int_TIME
~~~~~~~~

.. code-block:: cpp

   struct int_TIME {
       int year, month, day;
       int hour, minute, second, millisecond;
   };

Configuração em ``main.cpp``
----------------------------

A struct ``CONFIG`` concentra os padrões da CLI:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Campo
     - Padrão
   * - ``spu_sp1``
     - ``/dev/modbus/spuChA``
   * - ``spu_sp2``
     - ``/dev/modbus/spuChB``
   * - ``plc_adress``
     - ``192.168.1.1:4840``
   * - ``plc_conv_file``
     - ``""``
   * - ``port_csv``
     - ``1234``
   * - ``error_interval_spu`` / ``error_interval_plc``
     - ``2``
   * - ``monitor_interval``
     - ``3``
   * - ``monitor_stdout``
     - ``false``
   * - ``close``
     - ``false`` (``true`` após ``--help`` / ``--version`` / ``--license``)

``configOptions(argc, argv)`` interpreta a linha de comando com
cxxopts.
