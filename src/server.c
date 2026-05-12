#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_CLIENTS 10

int get_listener_fd(char* port) {
    struct addrinfo* res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // listen to port 6767 on localhost
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        char* err_str;
        sprintf(err_str, "error getting address info for port: %s on localhost!\n", port);
        perror(err_str);
        exit(EXIT_FAILURE);
    }

    int temp_fd = -1;
    struct addrinfo* p = res;
    while (p != NULL && temp_fd == -1) {
        temp_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (temp_fd != -1) break;
        p = p->ai_next;
    }
    if (temp_fd == -1) {
        perror("error starting a socket!\n");
        exit(EXIT_FAILURE);
    }
    if (bind(temp_fd, (struct sockaddr*)p->ai_addr, p->ai_addrlen) == -1) {
        perror("error binding socket to port!\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    if (listen(temp_fd, MAX_CLIENTS) == -1) { // MAX_CLIENTS is the limit of the number of incoming connections
        perror("error trying to set server to listen for incoming connections.\n");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port: %s\n", port);

    return temp_fd;
}

void handle_clients(int listener_fd, struct pollfd* pollfds, int idx, int* curr_clients) {
    if (*curr_clients >= MAX_CLIENTS) {
        // connect only to send an error message
        int err_fd = accept(listener_fd, NULL, NULL);
        if (err_fd == -1) return;
        char* err_str;
        sprintf(err_str, "Maximum number of clients (%d) reached!\n", MAX_CLIENTS);
        send(err_fd, err_str, strlen(err_str), 0);
        close(err_fd);
    }
    else {
        // connect normally
        printf("hahahah");
        int new_fd = accept(listener_fd, NULL, NULL);
        if (new_fd == -1) return;
        pollfds[++(*curr_clients)].fd = new_fd;
        pollfds[*curr_clients].events = POLLIN;
    }
}

void handle_messages(struct pollfd* pollfds, int idx, int* curr_clients) {
    if (pollfds[idx].revents & POLLHUP) {
        // connection closed
        pollfds[idx] = pollfds[--(*curr_clients)];
        return;
    }

    int target_fd = pollfds[idx].fd;
    char buffer[BUFSIZ];

    // receive the str from the client
    recv(target_fd, buffer, BUFSIZ, 0);

    for (int i = 1; i < *curr_clients + 1; i++) {
        if (i == idx) continue; // don't want to send the same message to the client twice
        send(target_fd, buffer, strlen(buffer), 0);
    }
}

void handle_polls(int listener_fd, struct pollfd* pollfds, int* curr_clients) {
    for (int i = 0; i < *curr_clients + 1; i++) {
        int event_occur = pollfds[i].revents & (POLLIN | POLLHUP);
        if (pollfds[i].fd == listener_fd)
            handle_clients(listener_fd, pollfds, i, curr_clients);
        else
            handle_messages(pollfds, i, curr_clients);
    }
}

int main(void) {
    int listener_fd = get_listener_fd("6767");
    printf("%d\n", listener_fd);
    int current_clients = 0;

    printf("%d\n", sizeof(struct pollfd));
    struct pollfd* pollfds = malloc(sizeof(struct pollfd) * (MAX_CLIENTS + 1));
    pollfds[0].fd = listener_fd;
    pollfds[0].events = POLLIN;

    while (1) {
        int num_events = poll(pollfds, current_clients + 1, 1000); // polls (returns) every second
        printf("%d\n", num_events);
        if (num_events > 0) {
            handle_polls(listener_fd, pollfds, &current_clients);
        }
    }

    free(pollfds);
}