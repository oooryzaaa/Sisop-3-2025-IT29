
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pwd.h>

#define MAX_ORDERS 100
#define SHM_KEY 1234

typedef struct {
    char name[64];
    char address[128];
    char type[16];
    int delivered;
    char agent[64];
} Order;

void write_log(const char* agent, const char* name, const char* address) {
    FILE* fp = fopen("delivery.log", "a");
    if (!fp) return;

    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    fprintf(fp, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Reguler package delivered to %s in %s\n",
        tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        agent, name, address);

    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s -list\n", argv[0]);
        printf("  %s -deliver [Nama]\n", argv[0]);
        printf("  %s -status [Nama]\n", argv[0]);
        return 1;
    }

    int shm_id = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, 0666);
    if (shm_id == -1) {
        perror("Shared memory not found");
        return 1;
    }

    Order* orders = (Order*) shmat(shm_id, NULL, 0);

    if (strcmp(argv[1], "-list") == 0) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strlen(orders[i].name) > 0) {
                printf("%s - %s\n", orders[i].name, orders[i].delivered ? "Delivered" : "Pending");
            }
        }
    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].name, argv[2]) == 0) {
                if (orders[i].delivered)
                    printf("Status for %s: Delivered by %s\n", argv[2], orders[i].agent);
                else
                    printf("Status for %s: Pending\n", argv[2]);
                break;
            }
        }
    } else if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].name, argv[2]) == 0 && orders[i].delivered == 0 && strcmp(orders[i].type, "Reguler") == 0) {
                orders[i].delivered = 1;
                struct passwd* pw = getpwuid(getuid());
                strcpy(orders[i].agent, pw->pw_name);
                write_log(orders[i].agent, orders[i].name, orders[i].address);
                printf("Order for %s delivered successfully.\n", argv[2]);
                break;
            }
        }
    } else {
        printf("Invalid command.\n");
    }

    shmdt(orders);
    return 0;
}
