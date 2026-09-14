Uso
===

Linha de comando
----------------

.. code-block:: text

   trigaserver [opções]

Opções informativas
~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Opção
     - Efeito
   * - ``-h`` / ``--help``
     - Mostra a ajuda e encerra.
   * - ``-v`` / ``--version``
     - Mostra a versão e encerra.
   * - ``-l`` / ``--license``
     - Mostra o aviso de licença e encerra.
   * - ``-m`` / ``--monitor``
     - Espelha o monitor de sistema na saída padrão (terminal).

Parâmetros de operação
~~~~~~~~~~~~~~~~~~~~~~

Todos os parâmetros abaixo também podem ser definidos em
``/etc/trigaserver.conf`` (veja :doc:`configuracao`). A linha de comando
sobrescreve o arquivo.

.. list-table::
   :header-rows: 1
   :widths: 28 22 50

   * - Opção
     - Padrão
     - Significado
   * - ``--SPU_CHA``
     - ``/dev/modbus/spuChA``
     - Dispositivo serial da SPU canal A.
   * - ``--SPU_CHB``
     - ``/dev/modbus/spuChB``
     - Dispositivo serial da SPU canal B.
   * - ``--PLC_ADRESS``
     - ``192.168.1.1:4840``
     - Endereço OPC UA do CLP no formato ``IP:PORTA``.
   * - ``--PLC_CONV_FILE``
     - *(vazio)*
     - Arquivo de fatores de conversão do CLP.
   * - ``--PORT_CSV``
     - ``1234``
     - Porta TCP do servidor CSV.
   * - ``--INTERVAL_ERROR_SPU``
     - ``2``
     - Segundos de espera antes de reconectar a SPU após erro.
   * - ``--INTERVAL_ERROR_PLC``
     - ``2``
     - Segundos de espera antes de reconectar o CLP após erro.
   * - ``--INTERVAL_MONITOR``
     - ``3``
     - Intervalo, em segundos, do monitor de sistema.

Exemplos
--------

Execução local com monitor no terminal:

.. code-block:: bash

   ./trigaserver --monitor \
       --SPU_CHA /dev/ttyUSB0 \
       --SPU_CHB /dev/ttyUSB1 \
       --PLC_ADRESS 192.168.1.1:4840 \
       --PLC_CONV_FILE /etc/libOpcTrigaPLC.conf \
       --PORT_CSV 1234

Consulta rápida sem iniciar o servidor:

.. code-block:: bash

   trigaserver --help
   trigaserver --version
   trigaserver --license

Monitor de sistema
------------------

O processo escreve periodicamente o estado das conexões e os valores
principais de potência (N) e período (T) em:

.. code-block:: text

   /tmp/trigaserver.systemmonitor

Com ``--monitor``, a mesma tela é reimpressa no terminal a cada
``INTERVAL_MONITOR`` segundos.

Campos exibidos:

* porta TCP do servidor CSV;
* ``STATE`` e endereço de SPU-A, SPU-B e CLP;
* potência e período lidos em cada hardware (no CLP, também a potência
  logarítmica).

O número de clientes conectados ainda **não** é monitorado.

Conectando um cliente
---------------------

O protocolo é descrito em :doc:`protocolo`. Exemplo mínimo em Python:

.. code-block:: python

   import socket

   host, port = "127.0.0.1", 1234
   intervalo_ms = 1000

   with socket.create_connection((host, port)) as s:
       s.sendall(f"{intervalo_ms}\n".encode())
       while True:
           dados = s.recv(8192)
           if not dados:
               break
           print(dados.decode(errors="replace"), end="")

A primeira linha recebida é o cabeçalho CSV. As seguintes são amostras.
