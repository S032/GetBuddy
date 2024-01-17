#include "erproc.h"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: ./client <IP_ADRRES>\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons(SERV_PORT);

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    printf("successfully connected\n");
    str_cli(stdin, sockfd);
}
