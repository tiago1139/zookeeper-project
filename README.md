# Projecto de Sistemas Distribuidos
O objetivo geral do projeto é concretizar um serviço de armazenamento de pares chavevalor (nos moldes da interface java.util.Map da API Java) similar ao utilizado pela Amazon para dar suporte aos seus serviços Web. Neste sentido, a estrutura de dados utilizada para armazenar esta informação é uma árvore de pesquisa binária, dada a sua elevada eficiência ao nível da pesquisa.

Este projeto simula o comportamento de um sistema distribuido em que há concorrência de vários pedidos simultâneos de vários clientes e existem vários servidores replicados para lidar com o fluxo de pedidos e tolerar falhas. Esta replicação segue o modelo Chain Replication e usa o ZooKeeper como serviço de coordenação entre os vários servidores.

É ainda usado o Protocol Buffer para serializar e de-serializar mensagens e estruturas complexas.

O projecto é desenvolvido em linguagem C usando o compilador GCC .

## Como executar o projecto:

1. Compilar o projecto
    1. Abrir o terminal na raiz do projecto
    2. Digitar make

2. Correr o executável
    1. Terminal no diretório binary
    2. Correr os testes
        - `./tree-server <port> <ip:port>(zookeeper)`
        - `./tree-client <host>:<port>(zookeeper)`

    3. Correr com Valgrind
        - `valgrind --leak-check=yes ./<programa>`


## Observações

1. Por alguma razão,muito muito raramente ao fazer CTRL+C num servidor ele não vai fechar até o cliente ser fechado.
