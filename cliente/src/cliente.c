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

    uint32_t seqn = 1;  // contador de requisições

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");  // servidor local por enquanto

    printf("Cliente iniciado.\n");
    printf("Digite comandos no formato: <IP destino> <valor>\n");
    printf("Exemplo: 127.0.0.2 15\n");
    printf("Digite CTRL+D ou CTRL+C para sair.\n\n");

    char ip_dest[32];
    uint32_t valor;

    while (scanf("%31s %u", ip_dest, &valor) == 2) {
        // Monta o pacote
        pacote.type = TYPE_REQ;
        pacote.seqn = seqn;
        pacote.data.req.dest_addr = inet_addr(ip_dest);
        pacote.data.req.value = valor;

        sendto(sock, &pacote, sizeof(packet_t), 0,
               (struct sockaddr *)&servidor, servidor_len);

        printf("[REQ %u] Enviada: destino=%s, valor=%u\n", seqn, ip_dest, valor);

        // Espera ACK
        memset(&resposta, 0, sizeof(packet_t));
        recvfrom(sock, &resposta, sizeof(packet_t), 0,
                 (struct sockaddr *)&servidor, &servidor_len);

        if (resposta.type == TYPE_REQ_ACK) {
            printf("[ACK %u] Novo saldo: %u\n\n",
                   resposta.seqn, resposta.data.ack.new_balance);
        }

        seqn++;  // incrementa ID para próxima requisição
    }

    printf("Encerrando cliente.\n");
    close(sock);
    return 0;
}
