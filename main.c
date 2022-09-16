#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

int MAXSIZE = 1000000;
typedef struct s_buffer{
	char buff[1000000];
	int	size;
} buffer;

typedef struct s_client {
    int    id;
    int    fd;
}    client;

int    sockfd, id, fd_max;
client clients[1000000];
fd_set memore_set, read_set, write_set;

void    putstr_fd(int fd, char *str) { int i = 0; while(str[i])i++; write(fd, str, i); }
void    fatal() { putstr_fd(STDERR_FILENO, "Fatal error\n"); exit(1); }

// server
int    startServ(int port) {
    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
    servaddr.sin_port = htons(port);

	for (int i = 0;i < MAXSIZE;i++) // initialize clients
		clients[i].id = clients[i].fd = -1;
    //try socket -> bind -> listen
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal();
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		fatal();
    if (listen(sockfd, 10) < 0)
    	fatal();
    return (sockfd);
}

// buffer functions
void	pushBack(buffer* self, char c) {
	self->buff[self->size] = c;
	self->size++;
}

void	concat(buffer* self, char* str) {
	int i = 0;
	while (str[i]) {
		pushBack(self, str[i]);
		i++;
	}
}

void	reset(buffer* self) {
	bzero(self->buff, sizeof(self->buff));
	self->size = 0;
}

void	dispatchMessage(buffer* message, int emitter) {
	for (int i = 0;i < MAXSIZE;i++) {
		if (clients[i].id != emitter && FD_ISSET(clients[i].fd, &write_set)) {
			send(clients[i].fd, message->buff, message->size, 0);
		}
	}
}

int	main(int argc, char **argv) {
	if (argc != 2) {
		putstr_fd(2, "Wrong number of arguments\n");
		exit(1);
	}
	fd_max = sockfd = startServ(atoi(argv[1]));
	FD_ZERO(&memore_set);
	FD_SET(sockfd, &memore_set);
	buffer clientJoin;
	buffer clientLeave;
	buffer clientMessage;
	while (1) {
		reset(&clientJoin); //we need all 3 buffers empty when nothing happens
		reset(&clientLeave);
		reset(&clientMessage);
		read_set = write_set = memore_set;
		if (select(fd_max + 1,&read_set, &write_set, NULL, NULL) < 0)
			fatal();
		if (FD_ISSET(sockfd, &read_set)) { // accept new client connection
			int slot = 0;
			while (slot < MAXSIZE && clients[slot].fd > 0) // get available client slot
				slot++;
			if ((clients[slot].fd = accept(sockfd, (void*)0, (void*)0)) < 0)
				fatal();
			clients[slot].id = id++; // global id incremented here
			FD_SET(clients[slot].fd, &memore_set); // add client fd to the set
			if (clients[slot].fd > fd_max) // update fd_max if needed
				fd_max = clients[slot].fd;
			clientJoin.size = sprintf(clientJoin.buff, "server: client %d just arrived\n", clients[slot].id);
			dispatchMessage(&clientJoin, clients[slot].id);
		}
		for (int slot = 0;slot < id;slot++) {
			if (clients[slot].fd < sockfd || !FD_ISSET(clients[slot].fd, &read_set))
				continue ;
			int recvRet = 1;
			while (recvRet == 1 && clientMessage.buff[clientMessage.size - 1] != '\n') {
				recvRet = recv(clients[slot].fd, clientMessage.buff + clientMessage.size, 1, 0);
				clientMessage.size += recvRet;
			}
			if (!recvRet) { // case client left
				clientLeave.size = sprintf(clientLeave.buff, "server: client %d just left\n", clients[slot].id);
				dispatchMessage(&clientLeave, clients[slot].id);
				close(clients[slot].fd);
				FD_CLR(clients[slot].fd, &memore_set);
				clients[slot].id = -1;
				clients[slot].fd = -1;
			} else {
				buffer tmp;
				buffer formatedMessage;
				reset(&tmp);
				reset(&formatedMessage);
				for (int i = 0;i < clientMessage.size;i++) {
					if (clientMessage.buff[i] == '\n') {
						char tmp2[100000] = {0};
						for (int i = 0; i < tmp.size && i < 100000;i++)
							tmp2[i] = tmp.buff[i];
						formatedMessage.size += sprintf(formatedMessage.buff + formatedMessage.size,
								"client %d: %s", clients[slot].id, tmp2);
					}
					else
						pushBack(&tmp, clientMessage.buff[i]);
				}
				dispatchMessage(&formatedMessage, clients[slot].id);
			}
		}
	}
	return (0);
}
