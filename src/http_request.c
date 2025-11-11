
// #define URL "http://ifconfig.me"
// #define HOSTNAME "ifconfig.me"

// #define URL "http://stockfish.online/api/s/v2.php"
// #define HOSTNAME "stockfish.online"


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <stdint.h>

#include <netdb.h>
#include "../include/chess.h"
#include "../include/chess_bot.h"
#include "../include/network.h"
#include "../include/chess_log.h"

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
 
    (void)data;
 
 
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
        CHESS_LOG(LOG_ERROR, "No such host\n");
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
        CHESS_LOG(LOG_ERROR, "SSL connection failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        return -1;
    }

    char request[1024];
    if (fast_strcmp(method, "GET") == 0) {
        snprintf(request, sizeof(request),
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: curl/7.81.0\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                "\r\n", http_request->endpoint, http_request->hostname);
    }

    if (SSL_write(ssl, request, ft_strlen(request)) <= 0) {
        CHESS_LOG(LOG_ERROR, "SSL write failed\n");
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
    
    (void)data;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct hostent *server = gethostbyname(http_request->hostname);
    if (server == NULL) {
        CHESS_LOG(LOG_ERROR, "No such host\n");
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

    if (fast_strcmp(method, "GET") == 0) {
        snprintf(request, sizeof(request),
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: curl/7.81.0\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                "\r\n", http_request->endpoint, http_request->hostname);
    }


    if (send(sock, request, ft_strlen(request), 0) < 0) {
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
        CHESS_LOG(LOG_ERROR, "Receive failed\n");
    }
}

int ft_isprintable(char c) {
    return (c >= 32 && c <= 126);
}

void hexa_dump_response(char *complete_response, size_t total_received) {
    CHESS_LOG(LOG_DEBUG, "Total bytes received: %zu\n", total_received);
    CHESS_LOG(LOG_DEBUG, "Hexa dump of response:\n");
    if (*get_chess_log_level() >= LOG_DEBUG) {
        for (size_t i = 0; i < total_received; i++) {
            char *color = GREEN;
            if ((unsigned char)complete_response[i] == '\n') {
                color = YELLOW;
            } else if ((unsigned char)complete_response[i] == '\r') {
                color = RED;
            }
            printf("%s%02x%s", color, (unsigned char)complete_response[i], RESET);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }

}

char *get_body(char *response, size_t response_len) {
    char *body_start = ft_strnstr(response, "\r\n\r\n", response_len) + 4;
    if (body_start) {
        size_t body_size = response_len - (body_start - response) + 1;
        
        CHESS_LOG(LOG_DEBUG, "Response length: %zu, Body size: %zu\n", response_len, body_size);
        CHESS_LOG(LOG_DEBUG, "Body starts: |%s|\n", body_start);
        
        char *body = ft_calloc(1, body_size);
        if (!body) {
            CHESS_LOG(LOG_ERROR, "Failed to allocate memory for body\n");
            return (NULL);
        }
        ft_memcpy(body, body_start, body_size);
        CHESS_LOG(LOG_DEBUG, "Extracted Body: |%s|\n", body);
        return (body);
    }
    return (NULL);
}

char *listen_https_response(HttpRequest *http_request) {
    if (!http_request->ssl_conn.ssl) {
        CHESS_LOG(LOG_ERROR, "No active HTTPS connection\n");
        return (NULL);
    }
    
    char buffer[4096] = {};
    int bytes_received = 0;


    char complete_response[8192] = {};
    size_t total_received = 0;

    while ((bytes_received = SSL_read(http_request->ssl_conn.ssl, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0';
        // printf("%s", buffer);
        if (total_received + bytes_received < sizeof(complete_response) - 1) {
            memcpy(complete_response + total_received, buffer, bytes_received);
            total_received += bytes_received;
        } else {
            CHESS_LOG(LOG_ERROR, "Response buffer overflow\n");
            break;
        }
        fast_bzero(buffer, sizeof(buffer));
    }

    if (total_received < 0) {
        CHESS_LOG(LOG_ERROR, "SSL read failed\n");
        ERR_print_errors_fp(stderr);
    }
    
    
    CHESS_LOG(LOG_INFO, "Complete HTTPS Response:\n%s\n", complete_response);
    hexa_dump_response(complete_response, total_received);

    char *body = get_body(complete_response, total_received);

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

    return (body);
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

    if (ft_strlen(url) < (size_t)i + 3) {
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
    CHESS_LOG(LOG_INFO, "URL scheme: %s\n", url);

    CHESS_LOG(LOG_INFO, "URL scheme extracted: %s\n", scheme);

    int is_http = fast_strcmp(scheme, "http") == 0;
    if (is_http) {
        return (0);
    }
    int is_https = fast_strcmp(scheme, "https") == 0;
    if (is_https) {
        return (1);
    }
    return (-1);
}

char *send_http_request(char *url) {

    HttpRequest http_request = {
        .hostname = "",
        .url = "",
        .endpoint = ""
    };


    if (!url) {
        CHESS_LOG(LOG_ERROR, "No URL provided\n");
        return (NULL);
    } else if (ft_strlen(url) >= 1024 || ft_strlen(url) == 0) {
        CHESS_LOG(LOG_ERROR, "URL too long or empty %zu\n", ft_strlen(url));
        return (NULL);
    }

    fast_strcpy(http_request.url, url);

    int use_https = is_https_url(http_request.url);

    if (use_https == -1) {
        CHESS_LOG(LOG_ERROR, "Invalid URL scheme. Use http:// or https://\n");
        return (NULL);
    } else if (use_https == 0) {
        CHESS_LOG(LOG_ERROR, "HTTP connection disabled\n");
        return (NULL);
    }

    extract_url_host(http_request.url, http_request.hostname);
    CHESS_LOG(LOG_INFO, "Extracted host: %s\n", http_request.hostname);

    extract_url_endpoint(http_request.url, http_request.endpoint);
    CHESS_LOG(LOG_INFO, "Extracted endpoint: %s\n", http_request.endpoint);

    if (ft_strlen(http_request.endpoint) == 0) {
        fast_strcpy(http_request.endpoint, "/");
        CHESS_LOG(LOG_INFO, "Defaulting endpoint to: %s\n", http_request.endpoint);
    }

    CHESS_LOG(LOG_INFO, "Using HTTPS connection...\n");
    int result = send_https_packet(&http_request, "GET", NULL);
    if (result > 0) {
        char *body = listen_https_response(&http_request);
        return (body);
    }
    CHESS_LOG(LOG_ERROR, "HTTP connection disabled\n");
    return (NULL);
}


