#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000
#define TAM_BUFFER 1024
#define MAX_CLIENTES 100
#define SALDO_INICIAL 100

// ---------- Estruturas auxiliares ----------
typedef struct {
    uint32_t address;   // IP do cliente (em inteiro)
    uint32_t last_req;  // último ID de requisição recebido
    uint32_t balance;   // saldo atual
} Cliente;

typedef struct {
    packet_t pacote;
    struct sockaddr_in cliente;
    int sock;
    socklen_t cliente_len;
} ThreadArgs;

// ---------- Tabela simples de clientes ----------
static Cliente clientes[MAX_CLIENTES];
static int num_clientes = 0;
static pthread_mutex_t mutex_tabela = PTHREAD_MUTEX_INITIALIZER;

// ---------- Funções auxiliares ----------

// Procura cliente pelo IP ou cria um novo
Cliente* obter_cliente(uint32_t address) {
    pthread_mutex_lock(&mutex_tabela);

    // já existe?
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].address == address) {
            pthread_mutex_unlock(&mutex_tabela);
            return &clientes[i];
        }
    }

    // cria novo
    if (num_clientes < MAX_CLIENTES) {
        clientes[num_clientes].address = address;
        clientes[num_clientes].last_req = 0;
        clientes[num_clientes].balance = SALDO_INICIAL;
        printf("Novo cliente adicionado (IP %u) com saldo inicial %d\n",
               address, SALDO_INICIAL);
        num_clientes++;
        pthread_mutex_unlock(&mutex_tabela);
        return &clientes[num_clientes - 1];
    }

    pthread_mutex_unlock(&mutex_tabela);
    return NULL; // tabela cheia
}

// Processa uma requisição dentro da thread
void *processa_requisicao(void *args) {
    ThreadArgs *dados = (ThreadArgs *)args;
    uint32_t ip_origem = dados->cliente.sin_addr.s_addr;

    Cliente *c = obter_cliente(ip_origem);
    if (!c) {
        printf("Erro: limite de clientes atingido.\n");
        free(dados);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&mutex_tabela);

    printf("Processando REQ id %u de %s (saldo atual %u)\n",
           dados->pacote.seqn, inet_ntoa(dados->cliente.sin_addr), c->balance);

    packet_t ack;
    ack.type = TYPE_REQ_ACK;
    ack.seqn = dados->pacote.seqn;

    // Verifica duplicata
    if (dados->pacote.seqn <= c->last_req) {
        printf("REQ duplicada detectada do cliente %s (última %u)\n",
               inet_ntoa(dados->cliente.sin_addr), c->last_req);
        ack.data.ack.new_balance = c->balance;
    }
    // Verifica saldo
    else if (c->balance < dados->pacote.data.req.value) {
        printf("Saldo insuficiente para cliente %s\n", inet_ntoa(dados->cliente.sin_addr));
        ack.data.ack.new_balance = c->balance;
    }
    else {
        // Processa transação
        c->balance -= dados->pacote.data.req.value;
        c->last_req = dados->pacote.seqn;
        printf("Transação OK -> novo saldo: %u\n", c->balance);
        ack.data.ack.new_balance = c->balance;
    }

    pthread_mutex_unlock(&mutex_tabela);

    sendto(dados->sock, &ack, sizeof(packet_t), 0,
           (struct sockaddr *)&dados->cliente, dados->cliente_len);

    free(dados);
    pthread_exit(NULL);
}

// ---------- Função principal ----------
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

    printf("Servidor pronto na porta %d...\n", ntohs(servidor.sin_port));

    while (1) {
        memset(&pacote, 0, sizeof(packet_t));
        recvfrom(sock, &pacote, sizeof(packet_t), 0,
                 (struct sockaddr *)&cliente, &cliente_len);

        if (pacote.type == TYPE_REQ) {
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->pacote = pacote;
            args->cliente = cliente;
            args->sock = sock;
            args->cliente_len = cliente_len;

            pthread_t tid;
            pthread_create(&tid, NULL, processa_requisicao, args);
            pthread_detach(tid);
        }
    }

    close(sock);
    return 0;
}
