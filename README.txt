Trabalho Prático – INF01151 – Etapa 1
Aluno: Patrick Alves de Queiros

Descrição geral:
Sistema cliente-servidor UDP que implementa:
  - Descoberta automática (broadcast)
  - Processamento de requisições REQ/ACK
  - Threads e exclusão mútua
  - Controle de duplicatas
  - Transferências entre clientes
  - Timeout e reenvio automático

Execução:
1. Abra um terminal e inicie o servidor:
   $ cd servidor
   $ make
   $ ./servidor 4000

2. Em outro terminal, inicie o cliente:
   $ cd cliente
   $ make
   $ ./cliente 4000

3. Faça transferências:
   127.0.0.2 10
   127.0.0.3 15

Dependências:
  - gcc
  - pthread
  - Linux ou WSL


