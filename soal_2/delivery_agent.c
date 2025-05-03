#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_ORDERS 100
#define SHM_KEY 1234

typedef struct {
    char name[64];
    char address[128];
    char type[16];
    int delivered;
    char agent[64];
} Order;

Order* orders;
int shm_id;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void write_log(const char* agent, const char* name, const char* address) {
    FILE* fp = fopen("delivery.log", "a");
    if (!fp) return;

    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    fprintf(fp, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
        tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        agent, name, address);

    fclose(fp);
}

void* agent_thread(void* arg) {
    char* agent_name = (char*) arg;

    while (1) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            pthread_mutex_lock(&lock);
            if (strcmp(orders[i].type, "Express") == 0 && orders[i].delivered == 0) {
                orders[i].delivered = 1;
                strncpy(orders[i].agent, agent_name, sizeof(orders[i].agent));
                write_log(agent_name, orders[i].name, orders[i].address);
                pthread_mutex_unlock(&lock);
                sleep(1);
                break;
            }
            pthread_mutex_unlock(&lock);
        }
        sleep(1);
    }
    return NULL;
}

void read_csv() {
    FILE* fp = fopen("delivery_order.csv", "r");
    if (!fp) {
        perror("CSV tidak ditemukan");
        exit(1);
    }

    char line[256];
    int index = 0;

    while (fgets(line, sizeof(line), fp) && index < MAX_ORDERS) {
        char* token = strtok(line, ",");
        if (!token) continue;
        strncpy(orders[index].name, token, sizeof(orders[index].name));

        token = strtok(NULL, ",");
        strncpy(orders[index].address, token, sizeof(orders[index].address));

        token = strtok(NULL, ",\n");
        strncpy(orders[index].type, token, sizeof(orders[index].type));

        orders[index].delivered = 0;
        strcpy(orders[index].agent, "-");
        index++;
    }

    fclose(fp);
}

int main() {
    shm_id = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, IPC_CREAT | 0666);
    orders = (Order*) shmat(shm_id, NULL, 0);

    read_csv();

    pthread_t a, b, c;
    pthread_create(&a, NULL, agent_thread, "AGENT A");
    pthread_create(&b, NULL, agent_thread, "AGENT B");
    pthread_create(&c, NULL, agent_thread, "AGENT C");

    pthread_join(a, NULL);
    pthread_join(b, NULL);
    pthread_join(c, NULL);

    shmdt(orders);
    return 0;
}

