#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000
#define MAX_CLIENTES 100
#define SALDO_INICIAL 100

typedef struct {
    uint32_t address;   // IP do cliente
    uint32_t last_req;  // último ID processado
    uint32_t balance;   // saldo atual
} Cliente;

typedef struct {
    packet_t pacote;
    struct sockaddr_in cliente;
    int sock;
    socklen_t cliente_len;
} ThreadArgs;

// ---------- Tabela de clientes ----------
static Cliente clientes[MAX_CLIENTES];
static int num_clientes = 0;
static pthread_mutex_t mutex_tabela = PTHREAD_MUTEX_INITIALIZER;

// Busca cliente ou cria se não existir
Cliente *obter_cliente(uint32_t address) {
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].address == address)
            return &clientes[i];
    }

    if (num_clientes < MAX_CLIENTES) {
        clientes[num_clientes].address = address;
        clientes[num_clientes].last_req = 0;
        clientes[num_clientes].balance = SALDO_INICIAL;
        printf("Novo cliente adicionado (IP %u) com saldo inicial %d\n",
               address, SALDO_INICIAL);
        num_clientes++;
        return &clientes[num_clientes - 1];
    }

    return NULL;
}

// ---------- Thread de processamento ----------
void *processa_requisicao(void *args) {
    ThreadArgs *dados = (ThreadArgs *)args;
    uint32_t ip_origem = dados->cliente.sin_addr.s_addr;
    uint32_t ip_destino = dados->pacote.data.req.dest_addr;
    uint32_t valor = dados->pacote.data.req.value;
    uint32_t id_req = dados->pacote.seqn;

    pthread_mutex_lock(&mutex_tabela);

    Cliente *origem = obter_cliente(ip_origem);
    Cliente *destino = obter_cliente(ip_destino);

    if (!origem || !destino) {
        printf("Erro: limite de clientes atingido.\n");
        pthread_mutex_unlock(&mutex_tabela);
        free(dados);
        pthread_exit(NULL);
    }

        printf("Cliente origem: %s | destino: %s | valor: %u\n",
           inet_ntoa(dados->cliente.sin_addr),
           inet_ntoa(*(struct in_addr *)&ip_destino),
           valor);


    packet_t ack;
    ack.type = TYPE_REQ_ACK;
    ack.seqn = id_req;

    // Evita duplicatas
    if (id_req <= origem->last_req) {
        printf("REQ duplicada de %s ignorada (última %u)\n",
               inet_ntoa(dados->cliente.sin_addr), origem->last_req);
        ack.data.ack.new_balance = origem->balance;
    }
    // Verifica saldo
    else if (origem->balance < valor) {
        printf("Saldo insuficiente para %s (saldo=%u, valor=%u)\n",
               inet_ntoa(dados->cliente.sin_addr), origem->balance, valor);
        ack.data.ack.new_balance = origem->balance;
    }
    // Faz a transferência
    else {
        origem->balance -= valor;
        destino->balance += valor;
        origem->last_req = id_req;
        printf("Transferência OK: %s enviou %u para %s\n",
               inet_ntoa(dados->cliente.sin_addr), valor, inet_ntoa(*(struct in_addr *)&ip_destino));
        printf("Novo saldo origem: %u | saldo destino: %u\n",
               origem->balance, destino->balance);
        ack.data.ack.new_balance = origem->balance;
    }

        pthread_mutex_unlock(&mutex_tabela);

    // ---- NOVO TRECHO ----
    printf("=== Estado atual dos clientes ===\n");
    for (int i = 0; i < num_clientes; i++) {
        struct in_addr addr;
        addr.s_addr = clientes[i].address;
        printf("Cliente %d -> IP: %s | Saldo: %u | Último ID: %u\n",
               i + 1, inet_ntoa(addr), clientes[i].balance, clientes[i].last_req);
    }
    printf("=================================\n");
    // ----------------------

}

// ---------- Main ----------
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
