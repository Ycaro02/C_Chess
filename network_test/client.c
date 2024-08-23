#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// #define SERVER_IP "86.209.1.28"  // Remplacez par votre IP publique
#define SERVER_IP "127.0.0.1"  // Remplacez par votre IP publique
#define SERVER_PORT 24242

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr, localaddr, peeraddr;
    char buffer[1024];
    socklen_t addr_len = sizeof(peeraddr);

    /* Create UDP socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

	int local_port = atoi(argv[1]);

	printf("Local port: %s:%d\n", SERVER_IP, local_port);

	/* Bind the socket */
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = INADDR_ANY;
    localaddr.sin_port = htons(local_port);
    if (bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

	/* Server addr configuration */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	/* Send a message to the server */
    sendto(sockfd, "Hello", strlen("Hello"), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

	/* Receive the message from the server */
    recvfrom(sockfd, &peeraddr, sizeof(peeraddr), 0, (struct sockaddr *)&servaddr, &addr_len);
    printf("Informations du pair reçues : %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	/* Send a message to the peer */
    sendto(sockfd, "Ping", strlen("Ping"), 0, (struct sockaddr *)&peeraddr, addr_len);
	
	/* Receive the message from the peer */
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
    printf("Réponse du pair : %s\n", buffer);

    close(sockfd);
    return 0;
}
