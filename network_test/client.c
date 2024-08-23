#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_PORT 24242
#define TEST_MSG_NB 3
#define MAX_ATTEMPTS 5
#define TIMEOUT_SEC 2

int main(int argc, char **argv) {
    struct sockaddr_in servaddr, localaddr, peeraddr;
    int sockfd;
    char buffer[1024];
    socklen_t addr_len = sizeof(peeraddr);
    int local_port = atoi(argv[1]);
    char server_ip[16];

    bzero(server_ip, 16);
    bzero(buffer, 1024);
    if (argc == 3) {
        strcpy(server_ip, argv[2]);
    } else {
        printf("Usage: %s <local_port> <server_ip>\n", argv[0]);
        return 1;
    } 

    /* Create UDP socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Server IP: %s, Local port : %d\n", server_ip, local_port);

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
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    /* Send a message to the server */
    sendto(sockfd, "Hello", strlen("Hello"), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    /* Receive the peer information */
    recvfrom(sockfd, &peeraddr, sizeof(peeraddr), 0, (struct sockaddr *)&servaddr, &addr_len);
    printf("Informations du pair reçues : %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

    /* Configure timeout for socket */
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Error setting socket timeout");
    }

    /* Send a few messages to the peer with sequence numbers */
    for (int i = 0; i < TEST_MSG_NB; i++) {
        int attempts = 0;
        int ack_received = 0;
        char msg[16];
        sprintf(msg, "Ping %d", i);

        while (attempts < MAX_ATTEMPTS && !ack_received) {
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&peeraddr, addr_len);
            printf("Message envoyé au pair : %s, tentative %d\n", msg, attempts + 1);

            // Wait for ACK
            int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
            if (recv_len > 0) {
                buffer[recv_len] = '\0';
                if (strcmp(buffer, "ACK") == 0) {
                    printf("ACK reçu pour message : %s\n", msg);
                    ack_received = 1;
                } else { // Other msg send ACK
					sendto(sockfd, "ACK", strlen("ACK"), 0, (struct sockaddr *)&peeraddr, addr_len);
					printf("Message reçu : %s -> Send ACK\n", buffer);
				}
            } else {
                printf("Pas d'ACK reçu, nouvelle tentative...\n");
            }
            attempts++;
			sleep(1);
		}

        if (!ack_received) {
            printf("Échec après %d tentatives pour le message : %s\n", MAX_ATTEMPTS, msg);
        }

        sleep(1);
    }

    /* Wait for Bye */
    // sendto(sockfd, "Bye", strlen("Bye"), 0, (struct sockaddr *)&peeraddr, addr_len);
    // while (1) {
    //     int len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &addr_len);
    //     if (len > 0) {
    //         buffer[len] = '\0';
    //         if (strcmp(buffer, "Bye") == 0) {
    //             break;
    //         }
    //     }
    // }

    close(sockfd);
    return 0;
}
