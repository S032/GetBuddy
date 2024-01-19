#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>

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

void err_sys(const char* err_text) {
    perror(err_text);
    exit(EXIT_FAILURE);
}

int Socket(int domain, int type, int protocol) {
    int res;
    if ((res = socket(domain, type, protocol)) < 0) err_sys("Socket Failed");
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res;
    if ((res = bind(sockfd, addr, addrlen)) < 0) err_sys("Bind Failed");
}

void Listen(int sockfd, int backlog) {
    int res;
    if ((res = listen(sockfd, backlog)) < 0) err_sys("Listen Failed");
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res;
    if ((res = accept(sockfd, addr, addrlen)) < 0) err_sys("Accept Failed");
    return res;
}

void Inet_pton(int af, const char *scr, void *dst) {
    int res = inet_pton(af, scr, dst);
    if (res == 0) err_sys("Addres is invalid");
    if (res == -1) err_sys("Adress Family is invalid");
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res;
    if ((res = connect(sockfd, addr, addrlen)) < 0) err_sys("Connect Failed");
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
    if ((PID = fork()) < 0) err_sys("Fork Failed: ");
    return PID;
}

typedef void Sigfunc(int);
Sigfunc* signalunv(int signum, Sigfunc *func) {
    struct sigaction act, oldact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signum, &act, &oldact) < 0)
        return (SIG_ERR);
    return (oldact.sa_handler);
}

void sig_chld(int signum) {
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

void str_serv(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

    while(1) {
        if ((n = readn(sockfd, buf, MAXLINE)) == 0)
            return;

        writen(sockfd, buf, MAXLINE);
    }
}

void str_cli(FILE *fp, int sockfd) {
    int maxfd;
    fd_set readset;
    char sendline[MAXLINE], recvline[MAXLINE];

    while(true) {
        if (fgets(sendline, MAXLINE, fp) == NULL)
            return;
        writen(sockfd, sendline, sizeof(sendline));
        if (readn(sockfd, recvline, MAXLINE) == 0)
            err_sys("server terminated prematurely");
        fputs(recvline, stdout);
    }
}

void str_cli_select(FILE *fp, int sockfd) {
    int maxfd;
    fd_set readset;
    char sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&readset);
    while(true) {
        FD_SET(fileno(fp), &readset);
        FD_SET(sockfd, &readset);
        maxfd = std::max(fileno(fp), sockfd) + 1;
        select(maxfd, &readset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readset)) { //socket's ready to read
            if (readn(sockfd, recvline, MAXLINE) == 0)
                err_sys("server terminated prematurely");
            fputs(recvline, stdout);
        }
        if (FD_ISSET(fileno(fp), &readset)) { //input device's ready to read
            if (fgets(sendline, MAXLINE, fp) == NULL)
                return; // all's done
            writen(sockfd, sendline, sizeof(sendline));
        }
    }
}
