#include <stdio.h>
#include <string.h>

#define MAX_WEAPONS 5

typedef struct {
    char name[30];
    int price;
    int damage;
    char passive[50];
} Weapon;

Weapon weapons[MAX_WEAPONS] = {
    {"Keris Sword", 50, 5, "-"},
    {"Fire Blade Azriel", 150, 10, "Burn: +2 dmg over time"},
    {"Ice Saber Amba", 200, 12, "Freeze: chance to skip enemy turn"},
    {"Steel Axe Jomok", 100, 8, "-"},
    {"Thunder Hammer", 250, 15, "Shock: +5 dmg on crit"}
};

void show_shop() {
    printf("\n=== Azriel's Weapon Shop ===\n");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        printf("%d. %s | Price: %d | Damage: %d | Passive: %s\n", i+1, weapons[i].name, weapons[i].price, weapons[i].damage, weapons[i].passive);
    }
    printf("===================\n");
}

Weapon get_weapon(int index) {
    if (index < 0 || index >= MAX_WEAPONS) {
        Weapon empty = {"", 0, 0, ""};
        return empty;
    }
    return weapons[index];
}
