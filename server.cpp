#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define MAXLINE 1024

int main() {
    SSL_CTX *ctx;
    SSL *ssl;
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buf[MAXLINE];

    // Initialize SSL library
    SSL_library_init();

    // Create SSL context
    ctx = SSL_CTX_new(TLS_server_method());

    // Load server certificate and key
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Error: could not load server certificate\n";
        return 1;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Error: could not load server key\n";
        return 1;
    }

    // Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // Bind socket to address and port
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8443);
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        std::cerr << "Error: could not bind socket\n";
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenfd, 10) != 0) {
        std::cerr << "Error: could not listen on socket\n";
        return 1;
    }

    std::cout << "Server listening on port 8443...\n";

    // Accept incoming connections and handle them
    while (true) {
        // Accept connection
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        // Create SSL object
        ssl = SSL_new(ctx);

        // Attach SSL object to socket
        SSL_set_fd(ssl, connfd);

        // Perform SSL handshake
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(connfd);
            continue;
        }

        // Receive data
        int n = SSL_read(ssl, buf, MAXLINE);
        if (n <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(connfd);
            continue;
        }

        // Print request
        std::cout << "Received request:\n";
        std::cout.write(buf, n);
        std::cout << std::endl;

        // Send response
        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello, world!";
        SSL_write(ssl, response, strlen(response));

        // Clean up SSL connection
        SSL_shutdown(ssl);
        SSL_free(ssl);

        // Close connection
        close(connfd);
    }

    // Clean up SSL context
    SSL_CTX_free(ctx);

    return 0;
}
