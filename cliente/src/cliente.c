#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA_PADRAO 4000
#define TAM_BUFFER 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servidor;
    char buffer[TAM_BUFFER];

    // 1. Cria socket UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(1);
    }

    // 2. Configura endereço do servidor
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(argc > 1 ? atoi(argv[1]) : PORTA_PADRAO);
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost

    // 3. Envia mensagem
    char msg[] = "Olá servidor!";
    sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&servidor, sizeof(servidor));

    // 4. Recebe resposta
    socklen_t serv_len = sizeof(servidor);
    recvfrom(sock, buffer, TAM_BUFFER, 0, (struct sockaddr *)&servidor, &serv_len);
    printf("Resposta do servidor: %s\n", buffer);

    close(sock);
    return 0;
}
