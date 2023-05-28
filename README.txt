Projecto 4 de Sistemas Distribuidos

Grupo 33
- Ricardo Soares fc54446
- Miguel Reis fc54409
- Tiago Pinto fc55172

Como executar o projecto:

1. Compilar o projecto
    1. Abrir o terminal na raiz do projecto
    2. Digitar make

2. Correr o executável
    1. Terminal no diretório binary
    2. Correr os testes
        - ./tree-server <port> <ip:port>(zookeeper)
        - ./tree-client <host>:<port>(zookeeper)

    3. Correr com Valgrind
        - valgrind --leak-check=yes ./<programa>


Observações

1. 
O projeto está a funcionar para o guião para os pedidos todos.

2.
Ao fazer as conexões entre os servidores às vezes se reutilizarmos uma porta a conexão não vai ser estabelecida.
Ao correr nos computadores do DI não tem ocorrido esta situação, mas nos nossos computadores pessoais tem, então 
achámos melhor deixar o aviso. Se os professores quiserem utilizar portas diferentes há uma probabilidade de ocorrer
erro de conexão, por isso pedimos para terem isso em consideração.

3.
Por alguma razão,muito muito raramente ao fazer CTRL+C num servidor ele não vai fechar até o cliente ser fechado.


4.
Não conseguimos arranjar uma maneira de obter o IP da máquina por código, por isso o nosso programa utiliza o 
endereço de loop.








