Instalação
==========

Dependências
------------

O TrigaServer precisa de:

* CMake ≥ 3.14 (o pacote Debian pede ≥ 3.30)
* Compilador C++20
* Boost (≥ 1.81, componente ``system``)
* cxxopts
* libmodbus
* libserialport
* jsoncpp
* pkg-config
* systemd (opcional, só para instalar a unit)

As bibliotecas internas ``libOpcTrigaPLC`` e ``libModbusSystematomSPU``
são compiladas junto com o projeto (subdiretório ``libs/``). A
``libOpcTrigaPLC`` baixa o cliente OPC UA `open62541pp` no configure do
CMake via FetchContent — a máquina de build precisa de rede, ``git`` e
``ca-certificates``.

Debian / Ubuntu
---------------

.. code-block:: bash

   sudo apt install git build-essential cmake libboost-all-dev \
       libcxxopts-dev libmodbus-dev libserialport-dev libjsoncpp-dev

Arch Linux
----------

.. code-block:: bash

   sudo pacman -Syu git base-devel cmake boost cxxopts libserialport
   paru -S libmodbus   # AUR

Compilação manual
-----------------

.. code-block:: bash

   git clone https://github.com/TRIGA-IPR-R1/TrigaServer.git
   cd TrigaServer
   cmake -B build
   cmake --build build -j$(nproc)

O binário fica em ``build/trigaserver``.

Pacote Debian (.deb)
--------------------

.. code-block:: bash

   git clone https://github.com/TRIGA-IPR-R1/TrigaServer.git
   cd TrigaServer
   cmake -B build
   cmake --build build -j$(nproc)
   cd build
   cpack -G DEB
   sudo apt install ./TrigaServer-*.deb

Ou, a partir da raiz do repositório, com ``dpkg-buildpackage``:

.. code-block:: bash

   sudo apt install build-essential debhelper devscripts
   dpkg-buildpackage -us -uc -b

O ``.deb`` é escrito no diretório pai da árvore de fontes.
A documentação HTML gerada pelo Sphinx fica em
``/usr/share/doc/trigaserver/html/``.

.. warning::

   O empacotamento Debian clona ``open62541pp`` durante o configure.
   Ambientes como sbuild/pbuilder precisam permitir acesso à rede no
   build. Veja ``debian/README.source``.

Pacote Arch (AUR / PKGBUILD)
----------------------------

.. code-block:: bash

   paru -S libmodbus
   mkdir -p ~/TrigaServer-PKG
   cd ~/TrigaServer-PKG
   wget https://raw.githubusercontent.com/TRIGA-IPR-R1/InstallGNU/ArchLinux/TrigaServer/PKGBUILD
   makepkg -si

Nix
---

O repositório traz um ``flake.nix``. Para entrar no ambiente de
desenvolvimento:

.. code-block:: bash

   nix develop

Para construir o pacote:

.. code-block:: bash

   nix build

O flake desativa a instalação do arquivo de configuração
(``-DDONT_INSTALL_CONF=1``) e aponta o CMake para os módulos das
bibliotecas SPU e PLC.

Permissão das portas seriais
----------------------------

O usuário que executa o TrigaServer precisa acessar as portas RS-485 das
SPUs. Em Debian, o grupo típico é ``dialout``; em Arch, ``uucp``:

.. code-block:: bash

   sudo usermod -aG dialout "$USER"   # Debian/Ubuntu
   sudo usermod -aG uucp "$USER"      # Arch

É necessário relogar para o grupo passar a valer.

Serviço systemd
---------------

A unit ``trigaserver.service`` é instalada, habilitada e iniciada junto
com o pacote Debian (equivalente a ``systemctl enable --now
trigaserver``). O processo roda como ``trigauser`` (criado via sysusers)
e lê ``/etc/trigaserver.conf``.

Revise a configuração se o hardware do sítio não estiver acessível; o
serviço tentará reconectar nos intervalos configurados.

.. code-block:: bash

   systemctl status trigaserver

Documentação HTTP (porta 8080)
------------------------------

O pacote Debian instala também o serviço ``trigaserver-doc.service``:
uma instância Apache só para o HTML, na porta **8080**. Ele é habilitado
e iniciado na instalação. O Apache do sistema (porta 80) não é usado.

.. code-block:: bash

   systemctl status trigaserver-doc
   xdg-open http://127.0.0.1:8080/

A raiz do site é ``/usr/share/doc/trigaserver/html/``. Logs em
``/var/log/trigaserver-doc/``. Configuração Apache em
``/etc/trigaserver/apache-doc.conf``.

Para desligar:

.. code-block:: bash

   sudo systemctl disable --now trigaserver-doc

