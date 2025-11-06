
// #define URL "http://ifconfig.me"
// #define HOSTNAME "ifconfig.me"

// #define URL "http://stockfish.online/api/s/v2.php"
// #define HOSTNAME "stockfish.online"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

char hostname[1024] = {};
char url[1024] = {};
char endpoint[1024] = {};

// Variables globales pour la connexion HTTPS active
static SSL *global_ssl = NULL;
static SSL_CTX *global_ctx = NULL;
static int global_socket = -1;

// Fonction pour initialiser OpenSSL
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Fonction pour nettoyer OpenSSL
void cleanup_openssl() {
    EVP_cleanup();
}

// Fonction pour créer le contexte SSL
SSL_CTX *create_ssl_context() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    // Configurer les options SSL pour améliorer la compatibilité
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); // Désactiver la vérification des certificats
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3); // Désactiver les anciens protocoles
    
    // Définir les suites de chiffrement compatibles
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!SRP:!CAMELLIA");
    
    return ctx;
}

int send_https_packet(const char *url, const char *method, const char *data) {
    init_openssl();
    
    // Créer le contexte SSL
    SSL_CTX *ctx = create_ssl_context();
    if (!ctx) {
        cleanup_openssl();
        return -1;
    }
    
    // Créer le socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "No such host\n");
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(443);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    
    // Activer SNI (Server Name Indication) - requis par beaucoup de serveurs modernes
    SSL_set_tlsext_host_name(ssl, hostname);

    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "SSL connection failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    char request[1024];
    if (strcmp(method, "GET") == 0) {
        snprintf(request, sizeof(request),
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: curl/7.81.0\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                "\r\n", endpoint, hostname);
    }

    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        fprintf(stderr, "SSL write failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    global_ssl = ssl;
    global_ctx = ctx;
    global_socket = sock;
    
    return 1; // Succès
}

int send_http_packet(const char *url, const char *method, const char *data) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "No such host\n");
        close(sock);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    char request[1024];

    if (strcmp(method, "GET") == 0) {
        snprintf(request, sizeof(request),
                "GET / HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: curl/7.81.0\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                 "\r\n", hostname);
    }


    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Send failed");
        close(sock);
        return -1;
    }
    return (sock);
}


void listen_http_response(int sock) {
    char buffer[4096];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        perror("Receive failed");
    }
}

void listen_https_response() {
    if (!global_ssl) {
        fprintf(stderr, "No active HTTPS connection\n");
        return;
    }
    
    char buffer[4096];
    int bytes_received;

    while ((bytes_received = SSL_read(global_ssl, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        fprintf(stderr, "SSL read failed\n");
        ERR_print_errors_fp(stderr);
    }
    
    // Nettoyer les ressources
    SSL_shutdown(global_ssl);
    SSL_free(global_ssl);
    close(global_socket);
    SSL_CTX_free(global_ctx);
    cleanup_openssl();
    
    // Remettre à zéro les variables globales
    global_ssl = NULL;
    global_ctx = NULL;
    global_socket = -1;
}

int main(int argc, char **argv) {

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <URL> <hostname> <endpoint> [https]\n", argv[0]);
        return 1;
    }

    strncpy(url, argv[1], sizeof(url) - 1);
    strncpy(hostname, argv[2], sizeof(hostname) - 1);
    strncpy(endpoint, argv[3], sizeof(endpoint) - 1);


    // Vérifier si HTTPS est demandé
    int use_https = (argc > 4 && strcmp(argv[4], "https") == 0);
    
    if (use_https) {
        printf("Using HTTPS connection...\n");
        int result = send_https_packet(url, "GET", NULL);
        if (result > 0) {
            listen_https_response();
        }
    } else {
        printf("Using HTTP connection...\n");
        int sock = send_http_packet(url, "GET", NULL);
        if (sock >= 0) {
            listen_http_response(sock);
            close(sock);
        }
    }
    return 0;
}