Configuração
============

Arquivo ``/etc/trigaserver.conf``
---------------------------------

Instalado a partir de ``resources/trigaserver.conf``. A unit systemd lê
esse arquivo como ``EnvironmentFile`` e passa cada variável como
argumento para ``trigaserver``.

.. code-block:: ini

   SPU_CHA=/dev/modbus/spuChA
   SPU_CHB=/dev/modbus/spuChB
   PLC_ADRESS=192.168.1.1:4840
   PLC_CONV_FILE=/etc/libOpcTrigaPLC.conf
   PORT_CSV=1234
   INTERVAL_ERROR_SPU=2
   INTERVAL_ERROR_PLC=2
   INTERVAL_MONITOR=3

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Variável
     - Papel
   * - ``SPU_CHA`` / ``SPU_CHB``
     - Caminho dos dispositivos seriais das SPUs. Em produção costumam
       ser nós estáveis em ``/dev/modbus/`` (udev).
   * - ``PLC_ADRESS``
     - Endpoint OPC UA ``IP:porta``. A porta padrão OPC UA é ``4840``.
   * - ``PLC_CONV_FILE``
     - Arquivo INI com os fatores de conversão das grandezas do CLP.
   * - ``PORT_CSV``
     - Porta TCP em que o servidor escuta clientes CSV.
   * - ``INTERVAL_ERROR_SPU`` / ``INTERVAL_ERROR_PLC``
     - Pausa, em segundos, entre uma falha de leitura e a tentativa de
       reconexão.
   * - ``INTERVAL_MONITOR``
     - Período de atualização do arquivo
       ``/tmp/trigaserver.systemmonitor``.

Ajuste os endereços ao layout da planta antes de habilitar o serviço.

Arquivo de conversão do CLP
---------------------------

Caminho típico: ``/etc/libOpcTrigaPLC.conf`` (cópia de
``libs/libOpcTrigaPLC/resources/libOpcTrigaPLC.conf``).

Cada seção corresponde a uma grandeza. Além dos fatores numéricos, a
chave ``unit`` declara a unidade de engenharia para a qual o valor
**convertido** está calibrado (por exemplo ``pcm``, ``W``, ``mRem/h``).
Deixe ``unit =`` vazio para grandeza adimensional (barras). Sem a chave,
a biblioteca usa o padrão interno da seção. O CSV publica essa unidade
no cabeçalho; o TrigaViewer só herda, não redefine.

O tipo de conversão depende do sensor:

.. list-table::
   :header-rows: 1
   :widths: 22 25 53

   * - Tipo
     - Parâmetros
     - Uso
   * - Todas
     - ``unit``
     - Unidade do valor CONV (vazia = adimensional).
   * - Linear (``CONV_LIN``)
     - ``x0``, ``x1``, ``y0``, ``y1``
     - Barras, canal linear, aquisições lineares, canal percentual.
   * - Logarítmico (``CONV_LOG``)
     - ``a``, ``b``
     - Canais log, sensores de radiação.
   * - Período (``CONV_PER``)
     - ``a``, ``b`` (``c``, ``d`` reservados)
     - Período dos canais log e de partida. ``a`` é a tensão associada
       a ±∞.
   * - Vazão (``CONV_VAZ``)
     - ``a``, ``b``, ``c``
     - Sensor de vazão do primário (placa de orifício). ``a``/``b``
       convertem mA em ΔP; ``c`` converte ΔP em vazão.

O CSV de saída traz **duas** cópias das grandezas do CLP:

* ``PLC_CONV_*`` — valores já convertidos para unidades de engenharia;
* ``PLC_ORIG_*`` — valores brutos lidos do servidor OPC UA.

Sem arquivo de conversão, a biblioteca usa fatores padrão internos
(identidade ou constantes de fallback). Sempre prefira um arquivo
calibrado para a planta.

Unit systemd
------------

Arquivo-fonte: ``resources/trigaserver.service``.

.. code-block:: ini

   [Unit]
   Description=TrigaServer
   After=network.target

   [Service]
   EnvironmentFile=/etc/trigaserver.conf
   ExecStart=.../trigaserver --SPU_CHA $SPU_CHA --SPU_CHB $SPU_CHB \
             --PLC_ADRESS $PLC_ADRESS --PLC_CONV_FILE $PLC_CONV_FILE \
             --PORT_CSV $PORT_CSV \
             --INTERVAL_ERROR_SPU $INTERVAL_ERROR_SPU \
             --INTERVAL_ERROR_PLC $INTERVAL_ERROR_PLC \
             --INTERVAL_MONITOR $INTERVAL_MONITOR
   Restart=always
   User=trigauser

   [Install]
   WantedBy=multi-user.target

``Restart=always`` relança o processo se ele cair. O pacote Debian
habilita e inicia a unit na instalação (``systemctl enable --now``).

Serviço da documentação (Apache)
--------------------------------

Arquivo-fonte: ``resources/trigaserver-doc.service`` e
``resources/trigaserver-doc.apache.conf``.

A instância escuta em ``0.0.0.0:8080`` e publica
``/usr/share/doc/trigaserver/html/``. No pacote Debian ela é habilitada
na instalação. Não altera o Apache do sistema (não ocupa a porta 80).

Configuração: ``/etc/trigaserver/apache-doc.conf``.
Logs: ``/var/log/trigaserver-doc/``.
