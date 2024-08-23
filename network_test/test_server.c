#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 24242
#define DISCONNECT_MSG "DISCONNECT"

typedef struct {
    struct sockaddr_in addr;
    int connected;
} ClientInfo;

void handle_client_message(int sockfd, ClientInfo *clientA, ClientInfo *clientB, struct sockaddr_in *cliaddr, char *buffer) {
    if (strcmp(buffer, DISCONNECT_MSG) == 0) {
        if (clientA->connected && memcmp(cliaddr, &clientA->addr, sizeof(struct sockaddr_in)) == 0) {
            clientA->connected = 0;
            printf("Client A disconnected: %s:%d\n", inet_ntoa(clientA->addr.sin_addr), ntohs(clientA->addr.sin_port));
        } else if (clientB->connected && memcmp(cliaddr, &clientB->addr, sizeof(struct sockaddr_in)) == 0) {
            clientB->connected = 0;
            printf("Client B disconnected: %s:%d\n", inet_ntoa(clientB->addr.sin_addr), ntohs(clientB->addr.sin_port));
        }
    } else {
        if (!clientA->connected) {
            clientA->addr = *cliaddr;
            clientA->connected = 1;
            printf("Client A connected: %s:%d\n", inet_ntoa(clientA->addr.sin_addr), ntohs(clientA->addr.sin_port));
        } else if (!clientB->connected) {
            clientB->addr = *cliaddr;
            clientB->connected = 1;
            printf("Client B connected: %s:%d\n", inet_ntoa(clientB->addr.sin_addr), ntohs(clientB->addr.sin_port));
        }

        if (clientA->connected && clientB->connected) {
            /* Send information from B to A */
            sendto(sockfd, &clientB->addr, sizeof(clientB->addr), 0, (struct sockaddr *)&clientA->addr, sizeof(clientA->addr));

            /* Send information from A to B */
            sendto(sockfd, &clientA->addr, sizeof(clientA->addr), 0, (struct sockaddr *)&clientB->addr, sizeof(clientB->addr));
        }
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buffer[1024];
    ClientInfo clientA = {0}, clientB = {0};

	bzero(buffer, 1024);

    /* Create UDP socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    /* Server addr configuration */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    /* Bind the socket */
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server waiting on port %d...\n", PORT);

    while (1) {
        int len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
        buffer[len] = '\0'; // Ensure null-terminated string
        printf("Message from %s:%d - %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), buffer);

        handle_client_message(sockfd, &clientA, &clientB, &cliaddr, buffer);
    }

    close(sockfd);
    return 0;
}