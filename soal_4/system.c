#include "shm_common.h"

struct SystemData *sys_ptr = NULL;

void init_system() {
    key_t sys_key = get_system_key();
    int shmid = shmget(sys_key, sizeof(struct SystemData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget(system)");
        exit(1);
    }

    sys_ptr = shmat(shmid, NULL, 0);
    if (sys_ptr == (void *)-1) {
        perror("shmat(system)");
        exit(1);
    }

    // Inisialisasi data jika pertama kali
    if (sys_ptr->num_hunters == 0 && sys_ptr->num_dungeons == 0) {
        sys_ptr->num_hunters = 0;
        sys_ptr->num_dungeons = 0;
        sys_ptr->current_notification_index = 0;
        printf("System initialized.\n");
    } else {
        printf("System loaded from shared memory.\n");
    }
}

void cleanup() {
    if (sys_ptr != NULL) {
        shmdt(sys_ptr);
    }
    key_t sys_key = get_system_key();
    int shmid = shmget(sys_key, sizeof(struct SystemData), 0666);
    if (shmid >= 0) {
        shmctl(shmid, IPC_RMID, NULL);
        printf("Shared memory removed.\n");
    }
}

void handle_signal(int sig) {
    printf("\nTerminating system...\n");
    cleanup();
    exit(0);
}

void display_hunters() {
    printf("=== HUNTER INFO ===\n");
    for (int i = 0; i < sys_ptr->num_hunters; i++) {
        struct Hunter h = sys_ptr->hunters[i];
        printf("Name: %s\tLevel: %d\tEXP: %d\tATK: %d\tHP: %d\tDEF: %d\tStatus: %s\n",
            h.username, h.level, h.exp, h.atk, h.hp, h.def, h.banned ? "BANNED" : "Active");
    }
}

void generate_dungeon() {
    if (sys_ptr->num_dungeons >= MAX_DUNGEONS) {
        printf("Dungeon limit reached!\n");
        return;
    }

    struct Dungeon d;
    sprintf(d.name, "Demon Castle");

    d.min_level = (rand() % 5) + 1;
    d.atk = (rand() % 51) + 100;
    d.hp = (rand() % 51) + 50;
    d.def = (rand() % 26) + 25;
    d.exp = (rand() % 151) + 150;

    // Generate shared memory key unik
    d.shm_key = rand();

    sys_ptr->dungeons[sys_ptr->num_dungeons++] = d;

    printf("Dungeon generated!\n");
    printf("Name: %s\n", d.name);
    printf("Minimum Level: %d\n", d.min_level);
}

void display_dungeons() {
    printf("=== DUNGEON INFO ===\n");
    for (int i = 0; i < sys_ptr->num_dungeons; i++) {
        struct Dungeon d = sys_ptr->dungeons[i];
        printf("[Dungeon %d]\n", i + 1);
        printf("Name: %s\n", d.name);
        printf("Minimum Level: %d\n", d.min_level);
        printf("EXP Reward: %d\n", d.exp);
        printf("ATK: %d\n", d.atk);
        printf("HP: %d\n", d.hp);
        printf("DEF: %d\n", d.def);
        printf("Key: %d\n", d.shm_key);
        printf("-------------------------\n");
    }
}

int main() {
    signal(SIGINT, handle_signal);  // Ctrl+C
    signal(SIGTERM, handle_signal); // Kill

    srand(time(NULL)); // untuk generate dungeon
    
    init_system();

    int choice;
    while (1) {
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Hunter Info\n");
        printf("2. Dungeon Info\n");
        printf("3. Generate Dungeon\n");
        printf("4. Ban Hunter\n");
        printf("5. Reset Hunter\n");
        printf("6. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                display_hunters();
                break;
            case 2:
                display_dungeons();
                break;
            case 3:
                generate_dungeon();
                break;
            case 6:
                handle_signal(0);
                break;
            default:
                printf("Not implemented yet.\n");
                break;
        }
    }
    return 0;
}
