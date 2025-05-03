

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void *handle_client(void *arg);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    pthread_t tid;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
    printf("[SERVER] Dungeon started on port %d\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        pthread_create(&tid, NULL, handle_client, (void *)&new_socket);
    }
    return 0;
}

void *handle_client(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) break;

        if (strncmp(buffer, "BATTLE", 6) == 0) {
            srand(time(0));
            int enemy_hp = rand() % 151 + 50; // 50-200 HP
            int reward = rand() % 101 + 50;   // 50-150 Gold

            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "ENEMY %d %d\n", enemy_hp, reward);
            send(sock, msg, strlen(msg), 0);
        }
    }
    close(sock);
    return NULL;
}
