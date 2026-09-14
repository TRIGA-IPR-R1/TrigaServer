Protocolo CSV/TCP
=================

Visão geral
-----------

O TrigaServer abre um socket TCP IPv4 (``AF_INET``, ``SOCK_STREAM``) em
``INADDR_ANY`` na porta configurada. A fila de conexões pendentes tem
tamanho 5. Cada cliente aceito ganha uma thread dedicada.

Handshake
---------

1. O cliente conecta.
2. O cliente envia **apenas dígitos** com o intervalo de amostragem em
   milissegundos, em geral terminado por ``\n``. Qualquer caractere não
   numérico (exceto o último byte recebido) faz o servidor fechar a
   conexão.
3. O servidor responde com **uma linha de cabeçalho CSV**.
4. Em seguida envia uma linha de valores a cada intervalo, até o cliente
   desconectar ou ``send()`` falhar.

Campos separados por ``;``. Cada linha termina em ``\n``.

Códigos de estado (``STATE``)
-----------------------------

Presentes em SPU-A, SPU-B e CLP:

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Valor
     - Significado
   * - ``-1``
     - Nenhuma tentativa de conexão ainda (valor inicial).
   * - ``0``
     - Leitura bem-sucedida.
   * - ``1``
     - Erro ao ler dados (SPU) ou erro de leitura (CLP).
   * - ``2``
     - Sem dispositivo/contexto Modbus (SPU) ou servidor OPC UA
       desconectado (CLP).

Grupos de colunas
-----------------

A linha CSV tem quatro blocos, nesta ordem:

1. **PLC convertido** (``PLC_CONV_*``)
2. **PLC original** (``PLC_ORIG_*``)
3. **SPU canal A** (``SPU_ChA_*``)
4. **SPU canal B** (``SPU_ChB_*``)

Cada bloco começa por ``STATE`` e pelo instante da leitura
(ano, mês, dia, hora, minuto, segundo, milissegundo).

Campos do CLP
~~~~~~~~~~~~~

Além de estado e tempo, cada bloco PLC contém:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Campo
     - Grandeza
   * - ``BarraReg``
     - Posição da barra de regulação.
   * - ``BarraCon``
     - Posição da barra de controle.
   * - ``BarraSeg``
     - Posição da barra de segurança.
   * - ``CLogALog``
     - Canal logarítmico — aquisição log.
   * - ``CLogALin``
     - Canal logarítmico — aquisição linear (potência).
   * - ``CLogAPer``
     - Canal logarítmico — período.
   * - ``CParALin``
     - Canal de partida — linear.
   * - ``CParALog``
     - Canal de partida — log.
   * - ``CParAPer``
     - Canal de partida — período.
   * - ``CLogARea``
     - Canal logarítmico — reativímetro.
   * - ``CLin``
     - Canal linear.
   * - ``CPer``
     - Canal percentual.
   * - ``SRadAre``
     - Sensor de radiação de área.
   * - ``SRadEntPri``
     - Sensor de radiação — entrada do primário.
   * - ``SRadPoc``
     - Sensor de radiação do poço.
   * - ``SRadRes``
     - Sensor de radiação das resinas.
   * - ``SRadSaiSec``
     - Sensor de radiação — saída do secundário.
   * - ``SRadAer``
     - Sensor de radiação de aerossóis.
   * - ``SVasPri``
     - Vazão do sistema primário de refrigeração.

Campos da SPU
~~~~~~~~~~~~~

Além de estado e tempo, cada canal (ChA / ChB) contém:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Campo
     - Grandeza
   * - ``N_DATA_FP``
     - Potência nuclear.
   * - ``T_DATA_FP``
     - Período.
   * - ``F1_DATA_FP`` / ``F2_DATA_FP`` / ``F3_DATA_FP``
     - Grandezas auxiliares da SPU (ponto flutuante).
   * - ``EMR_N_THRESHOLD`` / ``WRN_N_THRESHOLD``
     - Limiares de emergência e aviso de potência.
   * - ``EMR_T_THRESHOLD`` / ``WRN_T_THRESHOLD``
     - Limiares de emergência e aviso de período.
   * - ``EMR_N`` / ``WRN_N`` / ``EMR_T`` / ``WRN_T``
     - Flags digitais correspondentes.
   * - ``R1`` / ``R2`` / ``R3``
     - Relés / estados digitais.
   * - ``RDY``
     - Pronto.
   * - ``TEST``
     - Modo de teste.
   * - ``XXXX``
     - Campo reservado da SPU.

Nomes no cabeçalho
------------------

O cabeçalho é gerado por ``FormatCSV::generateCSVHeader()``. Exemplos
de nomes:

* ``PLC_CONV_CLogALin``
* ``PLC_ORIG_BarraReg``
* ``SPU_ChA_N_DATA_FP``
* ``SPU_ChB_STATE``

Inteiros saem com ``%d``; pontos flutuantes com ``%f``.

Cliente de teste com netcat
---------------------------

.. code-block:: bash

   printf '500\n' | nc 127.0.0.1 1234

Isso pede uma amostra a cada 500 ms. Interrompa com Ctrl+C.
