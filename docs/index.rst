TrigaServer
===========

**TrigaServer** é um software para sistemas GNU/Linux que adquire, em tempo
real, as grandezas do reator nuclear TRIGA IPR-R1 (CDTN/CNEN) e as
disponibiliza na rede no formato CSV sobre TCP.

O programa lê continuamente:

* as duas unidades SPU SystemAtom (canais A e B) via **Modbus-RTU** (RS-485);
* o CLP da mesa de controle via **OPC UA** (Ethernet).

Os valores são reunidos numa linha CSV e enviados a cada cliente TCP na taxa
de amostragem que o próprio cliente solicita.

.. note::

   Este software **não** controla o reator. Ele apenas lê grandezas já
   disponibilizadas pelos equipamentos de instrumentação e as redistribui
   na rede.

Conteúdo
--------

.. toctree::
   :maxdepth: 2
   :caption: Manual

   introducao
   instalacao
   uso
   configuracao
   protocolo
   arquitetura
   bibliotecas
   api
   licenca

Como gerar esta documentação
----------------------------

A partir da pasta ``docs/``::

   pip install -r requirements.txt
   make html        # HTML em _build/html/index.html
   make latexpdf    # PDF em _build/latex/TrigaServer.pdf
   make epub        # EPUB em _build/epub/TrigaServer.epub
   make man         # página man em _build/man/trigaserver.1
   make text        # texto puro em _build/text/

``make help`` lista todos os alvos disponíveis. ``make clean`` apaga
``_build/``.

No pacote Debian a mesma documentação HTML é publicada em
http://localhost:8080/ pelo serviço ``trigaserver-doc`` (veja
:doc:`instalacao`).

Índices
-------

* :ref:`genindex`
* :ref:`search`
