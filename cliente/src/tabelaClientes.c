#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../../include/mensagens.h"

#define MAX_CLIENTES 100
#define SALDO_INICIAL 100

typedef struct {
    uint32_t address;  // IP em formato inteiro
    uint32_t last_req; // Último ID processado
    uint32_t balance;  // Saldo atual
} Cliente;

static Cliente clientes[MAX_CLIENTES];
static int num_clientes = 0;
static pthread_mutex_t mutex_tabela = PTHREAD_MUTEX_INITIALIZER;

// Busca cliente por IP; se não existir, cria novo
Cliente* obter_cliente(uint32_t address) {
    pthread_mutex_lock(&mutex_tabela);

    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].address == address) {
            pthread_mutex_unlock(&mutex_tabela);
            return &clientes[i];
        }
    }

    if (num_clientes < MAX_CLIENTES) {
        clientes[num_clientes].address = address;
        clientes[num_clientes].last_req = 0;
        clientes[num_clientes].balance = SALDO_INICIAL;
        printf("Novo cliente adicionado com saldo inicial %d (IP: %u)\n", SALDO_INICIAL, address);
        num_clientes++;
        pthread_mutex_unlock(&mutex_tabela);
        return &clientes[num_clientes - 1];
    }

    pthread_mutex_unlock(&mutex_tabela);
    return NULL; // Tabela cheia
}

// Processa uma transação (simples)
int processar_transacao(uint32_t ip_origem, uint32_t id_req, uint32_t valor) {
    pthread_mutex_lock(&mutex_tabela);

    Cliente* c = NULL;
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].address == ip_origem) {
            c = &clientes[i];
            break;
        }
    }

    if (c == NULL) { // Novo cliente
        pthread_mutex_unlock(&mutex_tabela);
        obter_cliente(ip_origem);
        pthread_mutex_lock(&mutex_tabela);
        c = &clientes[num_clientes - 1];
    }

    // Verifica duplicata
    if (id_req <= c->last_req) {
        printf("REQ duplicada detectada do cliente %u (última=%u)\n", ip_origem, c->last_req);
        pthread_mutex_unlock(&mutex_tabela);
        return -1; // Duplicata
    }

    // Atualiza saldo
    if (c->balance < valor) {
        printf("Saldo insuficiente para cliente %u\n", ip_origem);
        pthread_mutex_unlock(&mutex_tabela);
        return -2; // Saldo insuficiente
    }

    c->balance -= valor;
    c->last_req = id_req;

    pthread_mutex_unlock(&mutex_tabela);
    return c->balance; // Novo saldo
}
