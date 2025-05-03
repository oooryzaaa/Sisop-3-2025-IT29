#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void log_activity(const char *ip, const char *message) {
    FILE *f = fopen("dungeon.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec, ip, message);

    fclose(f);
}

void *handle_client(void *arg) {
    int sock = *(int *)arg;
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(sock, (struct sockaddr *)&client_addr, &len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Log connection
    char join_msg[128];
    snprintf(join_msg, sizeof(join_msg), "Player has joined the server.");
    log_activity(client_ip, join_msg);
    printf("[INFO] %s\n", join_msg);

    char buffer[BUFFER_SIZE];
    ssize_t valread;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) break;

        if (strncmp(buffer, "BATTLE", 6) == 0) {
            srand(time(NULL) ^ pthread_self());
            int enemy_hp = rand() % 151 + 50;
            int reward = rand() % 101 + 50;

            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "ENEMY %d %d\n", enemy_hp, reward);
            send(sock, msg, strlen(msg), 0);

            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Action: BATTLE -> ENEMY %d HP, %d GOLD", enemy_hp, reward);
            log_activity(client_ip, log_msg);
        } else {
            const char *msg = "ERROR: Unknown request.\n";
            send(sock, msg, strlen(msg), 0);
        }
    }

    close(sock);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    pthread_t tid;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Dungeon started on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        int *client_sock = malloc(sizeof(int));
        *client_sock = new_socket;

        if (pthread_create(&tid, NULL, handle_client, client_sock) != 0) {
            perror("Thread creation failed");
            close(new_socket);
            free(client_sock);
        } else {
            pthread_detach(tid);
        }
    }

    close(server_fd);
    return 0;
}
