

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "shop.c"

#define PORT 9000
#define BUFFER_SIZE 1024

typedef struct {
    int gold;
    Weapon inventory[10];
    int inv_count;
    Weapon equipped;
    int kill_count;
} Player;

Player player = {.gold = 300, .inv_count = 0, .kill_count = 0};

void connect_and_battle();
void show_stats();
void show_inventory();
void shop_menu();

int main() {
    int choice;
    while (1) {
        printf("\n== Lost Dungeon Menu ==\n");
        printf("1. Show Player Stats\n2. View Inventory\n3. Weapon Shop\n4. Battle Mode\n5. Exit\n>> ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: show_stats(); break;
            case 2: show_inventory(); break;
            case 3: shop_menu(); break;
            case 4: connect_and_battle(); break;
            case 5: exit(0);
            default: printf("Invalid option.\n");
        }
    }
}

void show_stats() {
    printf("\n== Player Stats ==\n");
    printf("Gold: %d\n", player.gold);
    printf("Weapon: %s\n", strlen(player.equipped.name) ? player.equipped.name : "None");
    printf("Base Damage: %d\n", player.equipped.damage);
    if (strlen(player.equipped.passive) && strcmp(player.equipped.passive, "-") != 0)
        printf("Passive: %s\n", player.equipped.passive);
    printf("Enemies Defeated: %d\n", player.kill_count);
}

void show_inventory() {
    printf("\n== Inventory ==\n");
    for (int i = 0; i < player.inv_count; i++) {
        printf("%d. %s | Dmg: %d | Passive: %s\n", i+1, player.inventory[i].name, player.inventory[i].damage, player.inventory[i].passive);
    }
    printf("Equip weapon number? (-1 to cancel): ");
    int x;
    scanf("%d", &x);
    if (x > 0 && x <= player.inv_count) {
        player.equipped = player.inventory[x-1];
        printf("Equipped %s!\n", player.equipped.name);
    }
}

void shop_menu() {
    show_shop();
    printf("Select weapon to buy (-1 to cancel): ");
    int sel;
    scanf("%d", &sel);
    sel--;
    if (sel >= 0 && sel < 5) {
        Weapon w = get_weapon(sel);
        if (player.gold >= w.price) {
            player.inventory[player.inv_count++] = w;
            player.gold -= w.price;
            printf("Bought %s!\n", w.name);
        } else {
            printf("Not enough gold!\n");
        }
    }
}

void connect_and_battle() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return;
    }

    send(sock, "BATTLE", strlen("BATTLE"), 0);
    char buffer[BUFFER_SIZE] = {0};
    read(sock, buffer, BUFFER_SIZE);

    int enemy_hp, reward;
    sscanf(buffer, "ENEMY %d %d", &enemy_hp, &reward);
    printf("\nEnemy Appeared! HP: %d\n", enemy_hp);

    srand(time(0));
    while (enemy_hp > 0) {
        printf("Type 'attack' to strike or 'exit' to flee: ");
        char input[10];
        scanf("%s", input);

        if (strcmp(input, "attack") == 0) {
            int dmg = player.equipped.damage + (rand() % 5);
            int crit = rand() % 100 < 20 ? 1 : 0;
            if (crit) dmg *= 2;
            enemy_hp -= dmg;
            printf("You dealt %d damage! %s\n", dmg, crit ? "[CRITICAL HIT!]" : "");
            if (strlen(player.equipped.passive) > 1 && strcmp(player.equipped.passive, "-") != 0)
                printf("Passive activated: %s\n", player.equipped.passive);

            if (enemy_hp <= 0) {
                printf("Enemy defeated! You gained %d gold.\n", reward);
                player.gold += reward;
                player.kill_count++;
                break;
            } else {
                printf("Enemy HP left: %d\n", enemy_hp);
            }
        } else if (strcmp(input, "exit") == 0) {
            printf("You fled the battle.\n");
            break;
        } else {
            printf("Unknown command.\n");
        }
    }
    close(sock);
}
