Arquitetura
===========

O processo é multithread. A thread ``main`` configura o programa, cria o
objeto ``TrigaServer`` (que já dispara as leituras) e em seguida o
servidor TCP. Depois entra no loop do monitor.

Diagrama de blocos
------------------

.. code-block:: text

   ┌─────────────┐  Modbus-RTU   ┌──────────────────────────┐
   │  SPU ChA    │──────────────▶│ readModbusRTU (thread)   │──┐
   └─────────────┘               └──────────────────────────┘  │
   ┌─────────────┐  Modbus-RTU   ┌──────────────────────────┐  │  atomic
   │  SPU ChB    │──────────────▶│ readModbusRTU (thread)   │──┼─ shared_ptr
   └─────────────┘               └──────────────────────────┘  │
   ┌─────────────┐  OPC UA       ┌──────────────────────────┐  │
   │  CLP        │──────────────▶│ readOpcTCP (thread)      │──┘
   └─────────────┘               └──────────────────────────┘
                                              │
                                              ▼
                               ┌─────────────────────────────┐
                               │  createServer (TCP :PORT)   │
                               │  1 thread / cliente         │
                               │  CSV na taxa pedida         │
                               └─────────────────────────────┘
                                              │
                                              ▼
                                        clientes de rede

Threads
-------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Thread
     - Função
   * - ``readModbusRTU`` (×2)
     - Lê ``get_all()`` da SPU correspondente e grava em
       ``data_global_spuCh[0|1]``. Em erro (``STATE ≠ 0``), espera
       ``errorIntervalSPU`` segundos e chama ``tryConnect()``.
   * - ``readOpcTCP``
     - Lê ``get_all()`` do CLP e grava em ``data_global_plc``. Em erro,
       espera ``errorIntervalPLC``. Só reconecta se ``STATE == 2``
       (desconexão).
   * - ``createServer``
     - ``bind`` / ``listen`` / ``accept`` na porta CSV. Para cada
       cliente, lança ``handleTCPClients``.
   * - ``handleTCPClients`` + lambda
     - Recebe o intervalo; a lambda envia o cabeçalho e, em loop,
       monta a linha CSV a partir dos ponteiros globais.
   * - ``main`` (monitor)
     - Atualiza ``/tmp/trigaserver.systemmonitor`` e, opcionalmente, o
       terminal.

Todas as threads de I/O são ``detach()``. O processo não tem condição
de parada implementada: o loop do monitor (e o do servidor) rodam até
o processo ser morto pelo systemd ou pelo usuário.

Compartilhamento de dados
-------------------------

Os últimos valores lidos ficam em:

* ``std::atomic<std::shared_ptr<SPU_DATA>> data_global_spuCh[2]``
* ``std::atomic<std::shared_ptr<PLC_DATA>> data_global_plc``

A thread de leitura faz ``store()``; as threads de cliente fazem
``load()`` e trabalham numa cópia local. Isso evita mutex nas amostras
e permite vários clientes lendo o mesmo instante sem bloquear a
aquisição.

Geração do CSV
--------------

``FormatCSV::generateCSVHeader()`` e ``generateCSVTemplate()`` constroem
cabeçalho e string de ``printf`` a partir das matrizes
``PLC_DATA_MATRIX`` e ``SPU_DATA_MATRIX`` (definidas nos headers das
bibliotecas). ``TrigaServer::genString()``:

1. decodifica os três ``time_point`` em ``int_TIME``;
2. converte o bloco PLC com ``plc.convAllData()``;
3. preenche o template com ``snprintf``.

Estrutura do código
-------------------

.. code-block:: text

   TrigaServer/
   ├── src/
   │   ├── main.cpp              CLI, CONFIG, monitor, SIGPIPE
   │   ├── TrigaServer.cpp       aquisição, TCP, CSV
   │   └── libFormatCSV.cpp      cabeçalho e template CSV
   ├── include/
   │   ├── TrigaServer.h
   │   └── libFormatCSV.h
   ├── libs/
   │   ├── libModbusSystematomSPU/
   │   └── libOpcTrigaPLC/
   ├── resources/
   │   ├── trigaserver.conf
   │   └── trigaserver.service
   └── debian/                   empacotamento .deb

Reconexão
---------

* **SPU:** qualquer ``STATE`` diferente de 0 dispara espera +
  ``tryConnect()``. A pausa ocorre **depois** de publicar o
  ``STATE`` no ponteiro global, para o monitor e os clientes verem o
  erro em vez de um valor antigo com ``STATE = 0``.
* **CLP:** a espera ocorre em qualquer erro; ``tryConnect()`` só se
  ``STATE == 2``.
