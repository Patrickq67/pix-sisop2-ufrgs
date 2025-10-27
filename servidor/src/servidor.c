#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000
#define TAM_BUFFER 1024

// Estrutura para passar dados à thread
typedef struct {
    packet_t pacote;
    struct sockaddr_in cliente;
    int sock;
    socklen_t cliente_len;
} ThreadArgs;

// Função executada pela thread
void *processa_requisicao(void *args) {
    ThreadArgs *dados = (ThreadArgs *)args;

    printf("Thread iniciada para requisição id %u do cliente %s\n",
           dados->pacote.seqn, inet_ntoa(dados->cliente.sin_addr));

    // Simula o processamento da transação
    sleep(1); // (só para ver que é paralelo)

    // Cria o ACK
    packet_t ack;
    ack.type = TYPE_REQ_ACK;
    ack.seqn = dados->pacote.seqn;
    ack.data.ack.new_balance = 100 - dados->pacote.data.req.value;

    sendto(dados->sock, &ack, sizeof(packet_t), 0,
           (struct sockaddr *)&dados->cliente, dados->cliente_len);

    printf("Thread finalizou requisição id %u -> saldo novo %u\n",
           ack.seqn, ack.data.ack.new_balance);

    free(dados);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servidor, cliente;
    socklen_t cliente_len = sizeof(cliente);
    packet_t pacote;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);

    if (bind(sock, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Erro no bind");
        close(sock);
        exit(1);
    }

    printf("Servidor aguardando requisições na porta %d...\n", ntohs(servidor.sin_port));

    while (1) {
        memset(&pacote, 0, sizeof(packet_t));
        recvfrom(sock, &pacote, sizeof(packet_t), 0, (struct sockaddr *)&cliente, &cliente_len);

        if (pacote.type == TYPE_REQ) {
            printf("REQ recebida: id %u, valor %u\n", pacote.seqn, pacote.data.req.value);

            // Cria argumentos para a nova thread
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->pacote = pacote;
            args->cliente = cliente;
            args->sock = sock;
            args->cliente_len = cliente_len;

            // Cria thread para processar
            pthread_t tid;
            if (pthread_create(&tid, NULL, processa_requisicao, args) != 0) {
                perror("Erro ao criar thread");
                free(args);
            } else {
                pthread_detach(tid); // Libera automaticamente após término
            }
        }
    }

    close(sock);
    return 0;
}
