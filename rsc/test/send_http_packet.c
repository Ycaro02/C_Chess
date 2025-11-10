
// #define URL "http://ifconfig.me"
// #define HOSTNAME "ifconfig.me"

// #define URL "http://stockfish.online/api/s/v2.php?fen="
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

typedef struct SSLConnection {
    SSL     *ssl;
    SSL_CTX *ctx;
    int     socket;
} SSLConnection;

typedef struct HttpRequest {
    char            hostname[1024];
    char            url[1024];
    char            endpoint[1024];
    SSLConnection   ssl_conn;
} HttpRequest;

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_ssl_context() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    // Configure SSL options for better compatibility
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); // Disable certificate verification (curl -k)
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3); // Disable old protocols
    
    // define compatible cipher suites
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!SRP:!CAMELLIA");
    
    return ctx;
}

int send_https_packet(HttpRequest *http_request, const char *method, const char *data) {
    init_openssl();
    
    SSL_CTX *ctx = create_ssl_context();
    if (!ctx) {
        cleanup_openssl();
        return -1;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    struct hostent *server = gethostbyname(http_request->hostname);
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
    
    // Active SNI (Server Name Indication) - required by many modern servers
    SSL_set_tlsext_host_name(ssl, http_request->hostname);

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
                "\r\n", http_request->endpoint, http_request->hostname);
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

    http_request->ssl_conn.ssl = ssl;
    http_request->ssl_conn.ctx = ctx;
    http_request->ssl_conn.socket = sock;
    
    return 1; // Success
}

int send_http_packet(HttpRequest *http_request, const char *method, const char *data) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct hostent *server = gethostbyname(http_request->hostname);
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
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: curl/7.81.0\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                "\r\n", http_request->endpoint, http_request->hostname);
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

void listen_https_response(HttpRequest *http_request) {
    if (!http_request->ssl_conn.ssl) {
        fprintf(stderr, "No active HTTPS connection\n");
        return;
    }
    
    char buffer[4096];
    int bytes_received;

    while ((bytes_received = SSL_read(http_request->ssl_conn.ssl, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        fprintf(stderr, "SSL read failed\n");
        ERR_print_errors_fp(stderr);
    }
    
    // clean up resources
    SSL_shutdown(http_request->ssl_conn.ssl);
    SSL_free(http_request->ssl_conn.ssl);
    close(http_request->ssl_conn.socket);
    SSL_CTX_free(http_request->ssl_conn.ctx);
    cleanup_openssl();
    
    // Reset ssl global variables
    http_request->ssl_conn.ssl = NULL;
    http_request->ssl_conn.ctx = NULL;
    http_request->ssl_conn.socket = -1;
}


void extract_url_scheme(char *url, char *scheme) {
    int i = 0;
    while (url[i] && url[i] != ':') {
        scheme[i] = url[i];
        i++;
    }
    scheme[i] = '\0';
} 

void extract_url_host(char *url, char *host) {
    int i = 0;
    int j = 0;

    // Skip the scheme
    while (url[i] && url[i] != ':') {
        i++;
    }

    if (strlen(url) < i + 3) {
        printf("Invalid URL format\n");
        host[0] = '\0';
        return;
    }

    i += 3; // Skip "://"

    // Extract the host
    while (url[i] && url[i] != '/' && url[i] != ':') {
        host[j++] = url[i++];
    }
    host[j] = '\0';
}

void extract_url_endpoint(char *url, char *endpoint) {
    int i = 0;
    int j = 0;

    // Skip the scheme
    while (url[i] && url[i] != ':') {
        i++;
    }

    i += 3; // Skip "://"

    // Skip the host
    while (url[i] && url[i] != '/') {
        i++;
    }

    // Extract the endpoint
    while (url[i]) {
        endpoint[j++] = url[i++];
    }
    endpoint[j] = '\0';
}

int is_https_url(char *url) {
    char scheme[16] = {};

    extract_url_scheme(url, scheme);
    printf("URL scheme: %s\n", url);

    printf("URL scheme extracted: %s\n", scheme);

    int is_http = strcmp(scheme, "http") == 0;
    if (is_http) {
        return (0);
    }
    int is_https = strcmp(scheme, "https") == 0;
    if (is_https) {
        return (1);
    }
    return (-1);
}

int main(int argc, char **argv) {

    HttpRequest http_request = {
        .hostname = "",
        .url = "",
        .endpoint = ""
    };

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    strncpy(http_request.url, argv[1], 1024 - 1);

    int use_https = is_https_url(http_request.url);

    if (use_https == -1) {
        fprintf(stderr, "Invalid URL scheme. Use http:// or https://\n");
        return 1;
    }

    extract_url_host(http_request.url, http_request.hostname);
    printf("Extracted host: %s\n", http_request.hostname);

    extract_url_endpoint(http_request.url, http_request.endpoint);
    printf("Extracted endpoint: %s\n", http_request.endpoint);

    if (strlen(http_request.endpoint) == 0) {
        strcpy(http_request.endpoint, "/");
        printf("Defaulting endpoint to: %s\n", http_request.endpoint);
    }

    if (use_https) {
        printf("Using HTTPS connection...\n");
        int result = send_https_packet(&http_request, "GET", NULL);
        if (result > 0) {
            listen_https_response(&http_request);
        }
    } else {
        printf("Using HTTP connection...\n");
        int sock = send_http_packet(&http_request, "GET", NULL);
        if (sock >= 0) {
            listen_http_response(sock);
            close(sock);
        }
    }
    return 0;
}


