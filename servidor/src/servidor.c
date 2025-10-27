#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA_PADRAO 4000
#define TAM_BUFFER 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servidor, cliente;
    char buffer[TAM_BUFFER];
    socklen_t cliente_len = sizeof(cliente);

    // 1. Cria socket UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    // 2. Configura endereço do servidor
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);

    // 3. Associa (bind)
    if (bind(sock, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Erro no bind");
        close(sock);
        exit(1);
    }

    printf("Servidor escutando na porta %d...\n", ntohs(servidor.sin_port));

    // 4. Recebe mensagens
    while (1) {
        memset(buffer, 0, TAM_BUFFER);
        recvfrom(sock, buffer, TAM_BUFFER, 0, (struct sockaddr *)&cliente, &cliente_len);
        printf("Mensagem recebida de %s:%d -> %s\n",
               inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port), buffer);

        // 5. Responde ao cliente
        char resposta[] = "ACK do servidor";
        sendto(sock, resposta, strlen(resposta), 0, (struct sockaddr *)&cliente, cliente_len);
    }

    close(sock);
    return 0;
}
