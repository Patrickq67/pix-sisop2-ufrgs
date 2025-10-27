#ifndef MENSAGENS_H
#define MENSAGENS_H

#include <stdint.h>

#define TYPE_DESC 1
#define TYPE_DESC_ACK 2
#define TYPE_REQ 3
#define TYPE_REQ_ACK 4

// Estrutura de uma requisição de transferência
struct requisicao {
    uint32_t dest_addr;  // IP destino (em formato inteiro)
    uint32_t value;      // Valor da transferência
};

// Estrutura do ACK de uma requisição
struct requisicao_ack {
    uint32_t seqn;         // ID da requisição confirmada
    uint32_t new_balance;  // Novo saldo do cliente origem
};

// Estrutura do pacote genérico
typedef struct packet {
    uint16_t type;         // Tipo de pacote (DESC, DESC_ACK, REQ, REQ_ACK)
    uint32_t seqn;         // Número de sequência (ID da requisição)
    union {
        struct requisicao req;
        struct requisicao_ack ack;
    } data;
} packet_t;

#endif
