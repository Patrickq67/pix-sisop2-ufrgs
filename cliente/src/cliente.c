#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servidor;
    packet_t pacote, resposta;
    socklen_t servidor_len = sizeof(servidor);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");  // servidor local por enquanto

    // Envia requisição
    pacote.type = TYPE_REQ;
    pacote.seqn = 1;
    pacote.data.req.dest_addr = inet_addr("127.0.0.2");  // endereço de destino simulado
    pacote.data.req.value = 10;

    sendto(sock, &pacote, sizeof(packet_t), 0, (struct sockaddr *)&servidor, servidor_len);
    printf("REQ enviada (id %u, destino %u, valor %u)\n", 
           pacote.seqn, pacote.data.req.dest_addr, pacote.data.req.value);

    // Aguarda ACK
    recvfrom(sock, &resposta, sizeof(packet_t), 0, (struct sockaddr *)&servidor, &servidor_len);

    if (resposta.type == TYPE_REQ_ACK) {
        printf("ACK recebido (id %u, novo saldo %u)\n", resposta.seqn, resposta.data.ack.new_balance);
    }

    close(sock);
    return 0;
}
