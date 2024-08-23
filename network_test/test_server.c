#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 24242

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buffer[1024];

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

    struct sockaddr_in clientA, clientB;
    int clientCount = 0;

    while (1) {
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
        printf("Client numero %d connecté : %s:%d\n", clientCount, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        clientCount++;

        if (clientCount == 1) {
            clientA = cliaddr;
        } else if (clientCount == 2) {
            clientB = cliaddr;

			/* Send information from B to A */
            sendto(sockfd, &clientB, sizeof(clientB), 0, (struct sockaddr *)&clientA, sizeof(clientA));

			/* Send information from A to B */
            sendto(sockfd, &clientA, sizeof(clientA), 0, (struct sockaddr *)&clientB, sizeof(clientB));

			/* Reset client count */
            clientCount = 0;
        }
    }

    close(sockfd);
    return 0;
}
