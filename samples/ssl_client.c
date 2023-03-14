#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>

int main() {
    SSL_library_init();

    const SSL_METHOD *my_ssl_method = TLS_client_method();
    
    SSL_CTX *my_ssl_context = SSL_CTX_new(my_ssl_method);
    if (my_ssl_context == NULL) {
        printf("ERROR to create SSL context.\n");
        return 1;
    }

    SSL *my_ssl = SSL_new(my_ssl_context);
    if (my_ssl == NULL) {
        printf("ERROR to create SSL.\n");
        return 1;
    }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("111.65.250.2");
    addr.sin_port = htons(443);

    connect(client, (struct sockaddr *)&addr, sizeof(addr));

    SSL_set_fd(my_ssl, client);

    if (SSL_connect(my_ssl) <= 0) {
        printf("ERROR to SSL connect.\n");
        return 1;
    }

    char buf[2048] = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
    SSL_write(my_ssl, buf, strlen(buf));

    int len = SSL_read(my_ssl, buf, sizeof(buf));
    if (len < sizeof(buf))
        buf[len] = 0;

    printf("Read %d bytes: %s\n", len, buf);

    SSL_shutdown(my_ssl);
    SSL_free(my_ssl);
    SSL_CTX_free(my_ssl_context);
    close(client);
}