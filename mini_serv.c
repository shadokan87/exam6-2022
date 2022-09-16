#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct s_client {
    int    id;
    int    fd;
}    client;

typedef struct s_buff {
        int    i;
} buff;

client clients[1024];
char buff[1000000], msg[1000000];
int    sockfd, id, fd_max;
fd_set memore_set, read_set, write_set;

void    putstr_fd(int fd, char str) { write(fd, str, strlen(str)); }
void    fatal() { putstr_fd(STDERR_FILENO, "Fatal error\n"); exit(1); }

int    startServ(int port) {
    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
    servaddr.sin_port = htons(port);

    //try socket -> bind -> listen
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (!(sockfd
        && bind(sockfd, (const struct sockaddr)&servaddr, sizeof(servaddr))
        && listen(sockfd, 10) > 0))
    fatal();
    return (sockfd);
}

int    main(int argc, char **argv) {
    if (argc != 2) {
        putstr_fd(2, "Wrong number of arguments\n");
        exit(1);
    }
    return (0);
}
