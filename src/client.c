#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>

int get_connection_fd(char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    struct addrinfo *res, *p;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("gai error!\n");
        exit(EXIT_FAILURE);
    }

    int fd = -1;
    p = res;

    while (p != NULL && fd == -1) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd != -1) break;
        p = p->ai_next;
    }
    if (fd == -1) {
        perror("error starting a socket!\n");
        exit(EXIT_FAILURE);
    }

    if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
        perror("error connecting to server!\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    return fd;
}

void handle_polls(int fd, struct pollfd* pollfds) {
    if (pollfds[0].revents & (POLLIN | POLLHUP)) { // stdin
        if (pollfds[0].revents == POLLHUP) exit(EXIT_FAILURE); // stdin failure
        char str[BUFSIZ];

        recv(0, str, BUFSIZ - 1, 0); // receive the full message

        if (send(fd, str, sizeof(str), 0) == -1) {
            perror("error sending message to server!\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (pollfds[1].revents == POLLHUP) exit(EXIT_FAILURE); // server closed connection
        char str[BUFSIZ];

        recv(fd, str, BUFSIZ, 0) > 0; // receive the full message

        printf("%s\n", str);
    }
}

int main() {
    int fd = get_connection_fd("6767");

    // stdin is port 0 in the terminal
    struct pollfd pollfds[2]; // stdin and connected socket
    pollfds[0].fd = 0; // stdin
    pollfds[0].events = POLLIN;

    pollfds[1].fd = fd; // connection to server
    pollfds[1].events = POLLIN;

    while (1) {
        int num_events = poll(pollfds, 2, 1000); // returns each second
        if (num_events > 0) {
            handle_polls(fd, pollfds);
        }
    }
}