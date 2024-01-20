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
    int     i, maxi, maxfd, listenfd, connfd, currentsockfd;
    int     sockcount, client[FD_SETSIZE];
    ssize_t n;
    fd_set  readset, master;
    char    buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    maxfd = listenfd; // initialisation
    maxi = -1; // index in array client[]
    FD_ZERO(&master);
    FD_SET(listenfd, &master);
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    while(true) {
        readset = master;
        sockcount = select(maxfd + 1, &readset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &readset)) { // connect with new client
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);


            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd; // save discriptor in client[]
                    break;
                }
            }
            if (i == FD_SETSIZE)
                err_sys("too many clients");

            FD_SET(connfd, &master); // add the new discriptor
            if (connfd > maxfd)
                maxfd = connfd; // for select
            if (i > maxi)
                maxi = i; // max index in client[]


            char ip[32];
            uint16_t port = htons(cliaddr.sin_port);
            inet_ntop(AF_INET, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
            printf("client is connected form <%s:%d>\n", ip, port);

            sockcount--;
            if (sockcount <= 0)
                continue; // there are no discriptors ready to read
        }
        for (i = 0; i <= maxi; i++) { // check each client on having data
            if ( (currentsockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(currentsockfd, &readset)) {
                if ((n = recv(currentsockfd, buf, MAXLINE, 0)) == 0) {
                    // close client *EOF
                    close(currentsockfd);
                    FD_CLR(currentsockfd, &master);
                    client[i] = -1;

                    getpeername(connfd, (SA *) &cliaddr, &clilen);
                    char ip[32];
                    uint16_t port = htons(cliaddr.sin_port);
                    inet_ntop(AF_INET, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
                    printf("client is disconected form <%s:%d>\n", ip, port);
                }

                int allsock;
                for (int z = 0; z <= maxi; z++) {
                    if ( (allsock = client[z]) < 0 )
                        continue;
                    if (allsock == currentsockfd)
                        continue;
                    if (FD_ISSET(allsock, &master)) {
                        send(allsock, buf, n, 0);
                    }
                }

                sockcount--;
                if (sockcount <= 0)
                    break; // there are no discriptors ready to read
            }
        }
    }
}
