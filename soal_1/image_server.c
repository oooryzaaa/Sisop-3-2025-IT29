#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define DATABASE_PATH "/mnt/c/Praktikum/Modul_3/server/database/"
#define LOG_FILE "/mnt/c/Praktikum/Modul_3/server/server.log"
#define CLIENT_SECRETS_PATH "/mnt/c/Praktikum/Modul_3/client/secrets/"

void write_log(const char* source, const char* action, const char* info) {
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    fprintf(log_fp, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
    fclose(log_fp);
}

void reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; ++i) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

int hex_to_bin(const char *hex, unsigned char *bin) {
    size_t len = strlen(hex);
    int count = 0;
    for (size_t i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2hhx", &bin[count++]);
    }
    return count;
}

char* process_and_save(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *content = malloc(size + 1);
    if (!content) {
        fclose(fp);
        return NULL;
    }

    if (fread(content, 1, size, fp) != size) {
        fclose(fp);
        free(content);
        return NULL;
    }
    content[size] = '\0';
    fclose(fp);

    reverse(content);

    unsigned char buffer[BUFFER_SIZE];
    int len = hex_to_bin(content, buffer);
    free(content);

    time_t now = time(NULL);
    static char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%ld.jpeg", DATABASE_PATH, now);

    FILE *jpeg = fopen(filepath, "wb");
    if (!jpeg) return NULL;

    fwrite(buffer, 1, len, jpeg);
    fclose(jpeg);

    return filepath;
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes <= 0) {
        close(client_fd);
        return;
    }

    if (strncmp(buffer, "DECRYPT_FILE:", 13) == 0) {
        const char *raw_filename = buffer + 13;
        write_log("Client", "DECRYPT", raw_filename);

        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "client/secrets/%s", raw_filename);

        char *result_path = process_and_save(fullpath);
        if (result_path) {
            char *fname = strrchr(result_path, '/') + 1;
            write_log("Server", "SAVE", fname);
            write(client_fd, fname, strlen(fname));
        } else {
            write_log("Server", "SAVE_FAIL", fullpath);
            write(client_fd, "ERROR_SAVE", 10);
        }
    }
    else if (strncmp(buffer, "DOWNLOAD:", 9) == 0) {
        const char *filename = buffer + 9;
        char path[256];
        snprintf(path, sizeof(path), "%s%s", DATABASE_PATH, filename);

        FILE *fp = fopen(path, "rb");
        if (!fp) {
            write_log("Server", "NOT_FOUND", filename);
            write(client_fd, "ERROR: File not found", 22);
            close(client_fd);
            return;
        }

        write_log("Client", "DOWNLOAD", filename);
        write_log("Server", "UPLOAD", filename);

        while (!feof(fp)) {
            int n = fread(buffer, 1, BUFFER_SIZE, fp);
            if (n > 0) write(client_fd, buffer, n);
        }
        fclose(fp);
    }
    else {
        write(client_fd, "ERROR: Invalid command", 23);
    }

    close(client_fd);
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    chdir("/");

    umask(0);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

int main() {
    daemonize();

    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    mkdir("server", 0755);
    mkdir(DATABASE_PATH, 0755);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server berjalan di port %d...\n", PORT);
    write_log("Server", "START", "Menunggu koneksi...");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        handle_client(client_fd);
    }

    return 0;
}
