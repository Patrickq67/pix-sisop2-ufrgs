#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "../../include/mensagens.h"

#define PORTA_PADRAO 4000
#define TIMEOUT_SEG 2          // tempo máximo de espera por ACK
#define MAX_TENTATIVAS 3       // máximo de reenvios

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servidor;
    packet_t pacote, resposta;
    socklen_t servidor_len = sizeof(servidor);
    uint32_t seqn = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    // Define timeout para recvfrom()
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEG;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Cliente iniciado com timeout de %d s (máx. %d tentativas)\n\n",
           TIMEOUT_SEG, MAX_TENTATIVAS);
    printf("Digite: <IP destino> <valor>\nExemplo: 127.0.0.2 15\n\n");

    char ip_dest[32];
    uint32_t valor;

    while (scanf("%31s %u", ip_dest, &valor) == 2) {
        pacote.type = TYPE_REQ;
        pacote.seqn = seqn;
        pacote.data.req.dest_addr = inet_addr(ip_dest);
        pacote.data.req.value = valor;

        int tentativas = 0;
        int ack_recebido = 0;

        while (tentativas < MAX_TENTATIVAS && !ack_recebido) {
            tentativas++;

            sendto(sock, &pacote, sizeof(packet_t), 0,
                   (struct sockaddr *)&servidor, servidor_len);
            printf("[REQ %u] Enviada (tentativa %d) -> destino=%s, valor=%u\n",
                   seqn, tentativas, ip_dest, valor);

            memset(&resposta, 0, sizeof(packet_t));
            ssize_t bytes = recvfrom(sock, &resposta, sizeof(packet_t), 0,
                                     (struct sockaddr *)&servidor, &servidor_len);

            if (bytes > 0 && resposta.type == TYPE_REQ_ACK && resposta.seqn == seqn) {
                printf("[ACK %u] Recebido: novo saldo %u\n\n",
                       resposta.seqn, resposta.data.ack.new_balance);
                ack_recebido = 1;
            } else {
                printf("⚠️  Timeout aguardando ACK %d s...\n", TIMEOUT_SEG);
            }
        }

        if (!ack_recebido) {
            printf("❌ Nenhum ACK após %d tentativas. Servidor indisponível.\n\n",
                   MAX_TENTATIVAS);
        }

        seqn++;
    }

    printf("Encerrando cliente.\n");
    close(sock);
    return 0;
}
