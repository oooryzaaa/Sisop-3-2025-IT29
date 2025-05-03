#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void error_msg(const char *msg) {
    printf("ERROR: %s\n", msg);
}

void write_log(const char* source, const char* action, const char* info) {
    FILE *log_fp = fopen("client.log", "a");
    if (!log_fp) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    fprintf(log_fp, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
    fclose(log_fp);
}

int connect_to_server() {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        return -1;

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;

    return sock;
}

void decrypt_file() {
    char filename[256];
    printf("Masukkan nama file teks (contoh: input_1.txt): ");
    scanf(" %[^\n]", filename);

    write_log("Client", "DECRYPT", "Text data");

    int sock = connect_to_server();
    if (sock < 0) {
        error_msg("Tidak dapat terhubung ke server");
        return;
    }

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DECRYPT_FILE:%s", filename);
    send(sock, request, strlen(request), 0);

    char response[128];
    int r = read(sock, response, sizeof(response) - 1);
    if (r > 0) {
        response[r] = '\0';
        printf("Respon Server: %s\n", response);
    } else {
        error_msg("Tidak ada respon dari server");
    }

    close(sock);
}

void download_file() {
    char filename[128];
    printf("Masukkan nama file jpeg (contoh: 1744401234.jpeg): ");
    scanf(" %[^\n]", filename);

    write_log("Client", "DOWNLOAD", filename);

    int sock = connect_to_server();
    if (sock < 0) {
        error_msg("Tidak dapat terhubung ke server");
        return;
    }

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DOWNLOAD:%s", filename);
    send(sock, request, strlen(request), 0);

    char save_path[256];
    snprintf(save_path, sizeof(save_path), "client/%s", filename);

    FILE *fp = fopen(save_path, "wb");
    if (!fp) {
        error_msg("Gagal membuat file hasil");
        close(sock);
        return;
    }

    char buffer[BUFFER_SIZE];
    int r;
    int first_chunk = 1;
    while ((r = read(sock, buffer, BUFFER_SIZE)) > 0) {
        if (first_chunk && strncmp(buffer, "ERROR", 5) == 0) {
            buffer[r] = '\0';
            error_msg(buffer);
            fclose(fp);
            remove(save_path);
            close(sock);
            return;
        }
        fwrite(buffer, 1, r, fp);
        first_chunk = 0;
    }

    printf("File berhasil disimpan: %s\n", save_path);
    fclose(fp);
    close(sock);
}

int main() {
    int pilih;
    do {
        printf("\n");
        printf("==========================================\n");
        printf("            RPC Client System           \n");
        printf("==========================================\n");
        printf(" 1. Masukkan File\n");
        printf(" 2. Install File dalam Bentuk Foto\n");
        printf(" 3. Exit\n");
        printf("------------------------------------------\n");
        printf(" Masukkan pilihan Anda [1-3]: ");
        scanf("%d", &pilih);

        switch (pilih) {
            case 1: decrypt_file(); break;
            case 2: download_file(); break;
            case 3:
                write_log("Client", "EXIT", "Client requested to exit");
                printf(" Keluar dari Sistem ya\n");
                break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilih != 3);

    return 0;
}
