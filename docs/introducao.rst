Introdução
==========

O que é o TrigaServer
---------------------

O TrigaServer é o servidor de dados do reator TRIGA IPR-R1. Ele fica
entre o hardware de instrumentação e as aplicações de supervisão,
registro ou análise que precisam das grandezas do reator em tempo real.

Fluxo resumido:

1. Três threads de leitura falam continuamente com SPU-A, SPU-B e CLP.
2. Os últimos valores lidos ficam em ponteiros atômicos compartilhados.
3. Um servidor TCP escuta na porta configurada (padrão ``1234``).
4. Cada cliente envia o intervalo de amostragem em milissegundos.
5. O servidor devolve o cabeçalho CSV uma vez e, em seguida, uma linha
   de valores a cada intervalo.

O reator TRIGA IPR-R1
---------------------

O IPR-R1 é um reator de pesquisa tipo TRIGA, operado pelo Centro de
Desenvolvimento da Tecnologia Nuclear (CDTN/CNEN), em Belo Horizonte.
A instrumentação relevante para este software é:

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Equipamento
     - Protocolo
     - Função
   * - SPU SystemAtom (ChA e ChB)
     - Modbus-RTU (RS-485)
     - Potência nuclear, período, limiares de emergência/aviso e estados
       digitais da cadeia nuclear.
   * - CLP da mesa de controle
     - OPC UA (Ethernet)
     - Posição das barras, canais linear/log/percentual/partida,
       sensores de radiação e vazão do primário.

Versão
------

A versão documentada aqui corresponde à declarada no ``CMakeLists.txt``:

* **Versão:** 0.0.2
* **Linguagem:** C++20
* **Repositório:** https://github.com/TRIGA-IPR-R1/TrigaServer

Papel no sistema
----------------

O TrigaServer é o ponto único de aquisição. Clientes (registradores,
painéis, ferramentas de análise) não precisam falar Modbus nem OPC UA:
basta abrir um socket TCP, pedir o intervalo desejado e consumir CSV.

Formatos RAW e JSON já existiram em versões anteriores e foram
retirados. O único formato de saída atual é CSV com campos separados
por ponto-e-vírgula.
