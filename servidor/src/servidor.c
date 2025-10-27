#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000
#define TAM_BUFFER 1024

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
            printf("REQ recebida de %s -> id %u, destino %u, valor %u\n",
                   inet_ntoa(cliente.sin_addr),
                   pacote.seqn,
                   pacote.data.req.dest_addr,
                   pacote.data.req.value);

            // Cria o ACK
            packet_t ack;
            ack.type = TYPE_REQ_ACK;
            ack.seqn = pacote.seqn;
            ack.data.ack.new_balance = 100 - pacote.data.req.value;  // exemplo simples

            sendto(sock, &ack, sizeof(packet_t), 0, (struct sockaddr *)&cliente, cliente_len);
        }
    }

    close(sock);
    return 0;
}
