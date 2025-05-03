#include "shm_common.h"

struct SystemData *sys_ptr = NULL;
struct Hunter     *me      = NULL;

void attach_system() {
    key_t sys_key = get_system_key();
    int   shmid   = shmget(sys_key, sizeof(struct SystemData), 0666);
    if (shmid < 0) {
        perror("shmget(system)");
        exit(1);
    }
    sys_ptr = shmat(shmid, NULL, 0);
    if (sys_ptr == (void*)-1) {
        perror("shmat(system)");
        exit(1);
    }
}

////////////////////////////////////////////////

void register_hunter() {
    if (sys_ptr->num_hunters >= MAX_HUNTERS) {
        printf("Max hunters reached!\n");
        return;
    }

    char username[50];
    printf("Username: ");
    scanf("%49s", username);

    for (int i = 0; i < sys_ptr->num_hunters; i++) {
        if (strcmp(sys_ptr->hunters[i].username, username) == 0) {
            printf("Username sudah terdaftar!\n");
            return;
        }
    }

    key_t hk = ftok("/tmp", username[0]);
    int shmid = shmget(hk, sizeof(struct Hunter), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget(hunter)");
        return;
    }

    struct Hunter *new_h;
    new_h = shmat(shmid, NULL, 0);
    if (new_h == (void*)-1) {
        perror("shmat(hunter)");
        return;
    }

    strcpy(new_h->username, username);
    new_h->level = 1;
    new_h->exp   = 0;
    new_h->atk   = 10;
    new_h->hp    = 100;
    new_h->def   = 5;
    new_h->banned = 0;
    new_h->shm_key = hk;

    sys_ptr->hunters[sys_ptr->num_hunters] = *new_h;
    sys_ptr->num_hunters++;

    shmdt(new_h);
    printf("Registration success!\n");
}

void login_hunter() {
    char username[50];
    printf("Username: ");
    scanf("%49s", username);

    int idx = -1;
    for (int i = 0; i < sys_ptr->num_hunters; i++) {
        if (strcmp(sys_ptr->hunters[i].username, username) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        printf("User tidak ditemukan.\n");
        return;
    }
    if (sys_ptr->hunters[idx].banned) {
        printf("User ini sedang dibanned.\n");
        return;
    }

    key_t hk = sys_ptr->hunters[idx].shm_key;
    int shmid = shmget(hk, sizeof(struct Hunter), 0666);
    struct Hunter *self = shmat(shmid, NULL, 0);
    if (self == (void*)-1) {
        perror("shmat(self)");
        return;
    }
    me = self;

    printf("Login success! Welcome, %s.\n\n", me->username);
    hunter_menu();
}

void hunter_menu() {
    int choice;
    do {
        printf("\n=== %s's MENU ===\n", me->username);
        printf("1. Dungeon List\n");
        printf("2. Dungeon Raid\n");
        printf("3. PvP Duels\n");
        printf("4. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: list_dungeons(sys_ptr, me); break;
            case 2: raid_dungeon(sys_ptr, me); break;
            case 3: pvp_duel(sys_ptr, me); break;
            case 4: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void list_dungeons(struct SystemData* system_data, struct Hunter* hunter) {
    printf("\n=== AVAILABLE DUNGEONS ===\n");
    for (int i = 0; i < system_data->num_dungeons; ++i) {
        if (hunter->level >= system_data->dungeons[i].min_level) {
            printf("%d. %s\t(Level %d+)\n", i + 1,
                   system_data->dungeons[i].name,
                   system_data->dungeons[i].min_level);
        }
    }
    printf("\nPress enter to continue...");
    getchar();
    getchar();
}

void raid_dungeon(struct SystemData* system_data, struct Hunter* hunter) {
    printf("\n=== RAIDABLE DUNGEONS ===\n");
    int available_idx[MAX_DUNGEONS];
    int count = 0;
    for (int i = 0; i < system_data->num_dungeons; ++i) {
        if (hunter->level >= system_data->dungeons[i].min_level) {
            printf("%d. %s\t(Level %d+)\n", count + 1,
                   system_data->dungeons[i].name,
                   system_data->dungeons[i].min_level);
            available_idx[count] = i;
            count++;
        }
    }

    if (count == 0) {
        printf("No dungeon available for your level.\n");
        return;
    }

    printf("Choose Dungeon: ");
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        return;
    }

    int idx = available_idx[choice - 1];
    struct Dungeon selected = system_data->dungeons[idx];

    hunter->atk += selected.atk;
    hunter->hp += selected.hp;
    hunter->def += selected.def;
    hunter->exp += selected.exp;

if (hunter->exp >= 500) {
        hunter->level++;
        hunter->exp = 0;
    }

    // Remove dungeon by shifting
    for (int i = idx; i < system_data->num_dungeons - 1; ++i) {
        system_data->dungeons[i] = system_data->dungeons[i + 1];
    }
    system_data->num_dungeons--;

    for (int i = 0; i < system_data->num_hunters; ++i) {
        if (strcmp(system_data->hunters[i].username, hunter->username) == 0) {
            system_data->hunters[i] = *hunter;
            break;
        }
    }

    printf("\nRaid success! Gained:\n");
    printf("ATK: %d\nHP: %d\nDEF: %d\nEXP: %d\n", selected.atk, selected.hp, selected.def, selected.exp);
    printf("\nPress enter to continue...");
    getchar();
    getchar();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void pvp_duel(struct SystemData* system_data, struct Hunter* hunter) {
    printf("\n=== CHOOSE AN OPPONENT ===\n");
    int indices[MAX_HUNTERS];
    int count = 0;

    for (int i = 0; i < system_data->num_hunters; i++) {
        struct Hunter h = system_data->hunters[i];
        if (strcmp(h.username, hunter->username) != 0 && !h.banned) {
            printf("%d. %s (LV %d, ATK %d, HP %d, DEF %d)\n", count + 1,
                   h.username, h.level, h.atk, h.hp, h.def);
            indices[count++] = i;
        }
    }

    if (count == 0) {
        printf("No opponents available.\n");
        return;
    }

    printf("Choose opponent: ");
    int choice;
    scanf("%d", &choice);
    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        return;
    }

    int opp_idx = indices[choice - 1];
    struct Hunter* opponent = &system_data->hunters[opp_idx];

    // Attach ke shared memory lawan
    int shmid = shmget(opponent->shm_key, sizeof(struct Hunter), 0666);
    if (shmid < 0) {
        perror("shmget(opponent)");
        return;
    }
    struct Hunter* opp_ptr = shmat(shmid, NULL, 0);
    if (opp_ptr == (void*)-1) {
        perror("shmat(opponent)");
        return;
    }

    int my_power = hunter->atk + hunter->hp + hunter->def;
    int opp_power = opp_ptr->atk + opp_ptr->hp + opp_ptr->def;

    printf("\nYour power: %d | Opponent power: %d\n", my_power, opp_power);

    if (my_power >= opp_power) {
        printf("You WIN!\n");

        hunter->atk += opp_ptr->atk;
        hunter->hp  += opp_ptr->hp;
        hunter->def += opp_ptr->def;

        // Update ke system
        for (int i = 0; i < system_data->num_hunters; ++i) {
            if (strcmp(system_data->hunters[i].username, hunter->username) == 0) {
                system_data->hunters[i] = *hunter;
                break;
            }
        }

        // Hapus opponent dari system
        for (int i = opp_idx; i < system_data->num_hunters - 1; ++i) {
            system_data->hunters[i] = system_data->hunters[i + 1];
        }
        system_data->num_hunters--;

        // Remove shared memory opponent
        shmdt(opp_ptr);
        shmctl(shmid, IPC_RMID, NULL);

        printf("Opponent defeated and removed from system.\n");
    } else {
        printf("You LOSE!\n");

        opp_ptr->atk += hunter->atk;
        opp_ptr->hp  += hunter->hp;
        opp_ptr->def += hunter->def;

        // Update ke system
        system_data->hunters[opp_idx] = *opp_ptr;

        // Hapus self dari system
        for (int i = 0; i < system_data->num_hunters; ++i) {
            if (strcmp(system_data->hunters[i].username, hunter->username) == 0) {
                for (int j = i; j < system_data->num_hunters - 1; ++j) {
                    system_data->hunters[j] = system_data->hunters[j + 1];
                }
                system_data->num_hunters--;
                break;
            }
        }

        // Remove shared memory self
        shmdt(hunter);  // detach hunter
        shmctl(shmget(hunter->shm_key, sizeof(struct Hunter), 0666), IPC_RMID, NULL);

        printf("You were defeated and removed from the system.\n");
        printf("Logging out...\n");
        exit(0);  // terminate program karena user sudah dihapus
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    attach_system();

    int choice;
    do {
        printf("=== HUNTER MENU ===\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch(choice) {
                case 1: register_hunter(); break;
                case 2: login_hunter(); break;
                case 3: break;
                default: printf("Pilihan tidak valid.\n");
        }
    } while(choice != 3);

    shmdt(sys_ptr);
    return 0;
}
