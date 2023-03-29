#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define MAXLINE 1024

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " hostname\n";
        return 1;
    }

    SSL_CTX *ctx;
    SSL *ssl;
    int sockfd;
    struct addrinfo hints, *result, *rp;
    char buf[MAXLINE];

    // Initialize SSL library
    SSL_library_init();

    // Create SSL context
    ctx = SSL_CTX_new(TLS_client_method());

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Resolve hostname
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], "8443", &hints, &result) != 0) {
        std::cerr << "Error: no such host\n";
        return 1;
    }

    // Connect to server
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
    }
    if (rp == NULL) {
        std::cerr << "Error: could not connect\n";
        return 1;
    }
    freeaddrinfo(result);

    // Create SSL object
    ssl = SSL_new(ctx);

    // Attach SSL object to socket
    SSL_set_fd(ssl, sockfd);

    // Perform SSL handshake
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        return 1;
    }

    // Send HTTP request
    char request[] = "GET / HTTP/1.1\r\nHost: ";
    strncat(request, argv[1], MAXLINE - strlen(request) - 3);
    strncat(request, "\r\n\r\n", MAXLINE - strlen(request) - 1);
    SSL_write(ssl, request, strlen(request));

    // Receive response
    int n = SSL_read(ssl, buf, MAXLINE);
    if (n <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        return 1;
    }

    // Print response
    std::cout << "Response:\n";
    std::cout.write(buf, n);
    std::cout << std::endl;

    // Clean up SSL connection
    SSL_shutdown(ssl);
    SSL_free(ssl);

    // Close connection
    close(sockfd);

    // Clean up SSL context
    SSL_CTX_free(ctx);

    return 0;
}
