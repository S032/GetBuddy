#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

/* Conection Constants */
#define LOCAL_IP "127.0.0.1"
#define SERV_IP "192.168.1.103"
#define SERV_PORT 34543

/* Max Constants */
#define LISTENQ 1024
#define MAXLINE 4096
#define BUFFSIZE 8192

/* Acronym */
#define SA  struct sockaddr

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res < 0) {
        perror("Socket Failed: ");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res < 0) {
        perror("Bind Failed: ");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res < 0) {
        perror("Listen Failed: ");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res < 0) {
        perror("Accept Failed: ");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Inet_pton(int af, const char *scr, void *dst) {
    int res = inet_pton(af, scr, dst);
    if (res == 0) {
        perror("Addres is invalid: ");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("Adress Family is invalid: ");
        exit(EXIT_FAILURE);
    }
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res < 0) {
        perror("Connect Failed: ");
        exit(EXIT_FAILURE);
    }
}

ssize_t readn(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char *) vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno = EINTR)
                nread = 0; //call read again*
            else
                return (-1);
        }
        else if (nread == 0) break; //EOF*

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft); // return postive value*
}

ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (const char *) vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno = EINTR)
                nwritten = 0; //call write again*
            else
                return (-1);
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

pid_t Fork(void) {
    pid_t PID;
    if ((PID = fork()) < 0) {
        perror("Fork Failed: ");
        exit(EXIT_FAILURE);
    }
    return PID;
}

void str_serv(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

    while(true) {
        if ((n = readn(sockfd, buf, MAXLINE)) == 0)
            return;

        printf("client: "), fputs(buf, stdout); 
    }
}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    while(fgets(sendline, MAXLINE, fp) != NULL) {
        writen(sockfd, sendline, MAXLINE);
    }
}
