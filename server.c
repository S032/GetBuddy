#include "erproc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    pid_t childpid;
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientinfo;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    socklen_t cadrlen = sizeof(clientinfo);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    signalunv(SIGCHLD, sig_chld);

    while (1) {
        if ((connfd = accept(listenfd, (SA *) &clientinfo, &cadrlen)) < 0) {
            if (errno == EINTR)
                continue;
            else
                err_sys("Accept Failed: ");
        }

        getsockname(connfd, (SA *) &clientinfo, &cadrlen);
        char ip[32];
        uint16_t port = htons(clientinfo.sin_port);
        inet_ntop(AF_INET, &clientinfo.sin_addr, ip, INET_ADDRSTRLEN);
        printf("[%s:%d] connected\n", ip, port);

        if ((childpid = Fork()) == 0) {
            close(listenfd);
            str_serv(connfd);
            printf("[%s:%d] disconnected\n", ip, port);
            close(connfd);
            exit(0);
        }

        close(connfd);
    }
}
