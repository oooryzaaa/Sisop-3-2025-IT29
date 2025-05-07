# Sisop-3-2025-IT29

## SOAL 2

Dikerjakan oleh : Angga Firmansyah

Tahun 2025, di tengah era perdagangan serba cepat, berdirilah
sebuah perusahaan ekspedisi baru bernama RushGo. RushGo ingin
memberikan layanan ekspedisi terbaik dengan 2 pilihan, Express
(super cepat) dan Reguler (standar). Namun, pesanan yang masuk
sangat banyak! Mereka butuh sebuah sistem otomatisasi
pengiriman, agar agen-agen mereka tidak kewalahan menangani
pesanan yang terus berdatangan. Kamu ditantang untuk
membangun Delivery Management System untuk RushGo
😆(Author: Nayla / naylaarr)

Sistem ini terdiri dari dua bagian utama:

- delivery_agent.c untuk agen otomatis pengantar Express
- dispatcher.c untuk pengiriman dan monitoring pesanan oleh
user

a. Mengunduh File Order dan Menyimpannya ke Shared Memory
Untuk memulai, Anda perlu mengelola semua orderan yang
masuk dengan menggunakan shared memory.

● Unduh file delivery_order.csv

● Setelah file CSV diunduh, program Anda harus
membaca seluruh data dari CSV dan menyimpannya ke
dalam shared memory.

b. Pengiriman Bertipe Express
RushGo memiliki tiga agen pengiriman utama: AGENT A,
AGENT B, dan AGENT C.

● Setiap agen dijalankan sebagai thread terpisah.

● Agen-agen ini akan secara otomatis:

○ Mencari order bertipe Express yang belum dikirim.

○ Mengambil dan mengirimkannya tanpa intervensi
user.

● Setelah sukses mengantar, program harus mencatat log
di delivery.log dengan format:
```bash
[dd/mm/yyyy hh:mm:ss] [AGENT A/B/C] Express package
delivered to [Nama] in [Alamat]
```
c. Pengiriman Bertipe Reguler
Berbeda dengan Express, untuk order bertipe Reguler,
pengiriman dilakukan secara manual oleh user.

● User dapat mengirim permintaan untuk mengantar
order Reguler dengan memberikan perintah deliver dari
dispatcher.

Penggunaan:
```bash
./dispatcher -deliver [Nama]
```
● Pengiriman dilakukan oleh agent baru yang namanya
adalah nama user.

● Setelah sukses mengantar, program harus mencatat log
di delivery.log dengan format:

```bash
[dd/mm/yyyy hh:mm:ss] [AGENT <user>] Reguler package
delivered to [Nama] in [Alamat
```

d. Mengecek Status Pesanan
Dispatcher juga harus bisa mengecek status setiap pesanan.
Penggunaan:
./dispatcher -status [Nama]

Contoh:
```bash
Status for Valin: Delivered by Agent C
Status for Novi: Pending
```
e. Melihat Daftar Semua Pesanan
Untuk memudahkan monitoring, program dispatcher bisa
menjalankan perintah list untuk melihat semua order disertai
nama dan statusnya.
Penggunaan:
```bash
./dispatcher -list
```

Penyelesaian : 

### dispatcher.c

```bash
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

```

MAX_ORDERS: Batas maksimal order yang dapat ditangani (100)

SHM_KEY: Key untuk shared memory (1234)

Struct Order: Merepresentasikan data pesanan dengan field:

name: Nama penerima (64 karakter)

address: Alamat pengiriman (128 karakter)

type: Jenis paket (16 karakter)

delivered: Status pengiriman (0/1)

agent: Nama agen pengirim (64 karakter)

## Fungsi Logging
```bash
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
Fungsi untuk mencatat log pengiriman ke file delivery.log

Format log: [timestamp] [agent] Reguler package delivered to [name] in [address]

Timestamp berisi tanggal dan waktu lengkap
```

### Fungsi Main
```bash
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s -list\n", argv[0]);
        printf("  %s -deliver [Nama]\n", argv[0]);
        printf("  %s -status [Nama]\n", argv[0]);
        return 1;
    }
Mengecek argumen command line

Menampilkan petunjuk penggunaan jika argumen tidak sesuai
```
### List Orders
```bash
    if (strcmp(argv[1], "-list") == 0) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strlen(orders[i].name) > 0) {
                printf("%s - %s\n", orders[i].name, orders[i].delivered ? "Delivered" : "Pending");
            }
        }
    }
```
### Menampilkan semua order yang ada
```bash
Format: [nama] - [status] (Delivered/Pending)
```
Hanya menampilkan order yang memiliki nama (tidak kosong)

## Check Status
```bash
    else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].name, argv[2]) == 0) {
                if (orders[i].delivered)
                    printf("Status for %s: Delivered by %s\n", argv[2], orders[i].agent);
                else
                    printf("Status for %s: Pending\n", argv[2]);
                break;
            }
        }
    }
```
Fungsinya untuk cek status order berdasarkan nama, jika ditemukan, tampilkan status dan nama agen (jika sudah delivered). Jika tidak ditemukan, tidak menampilkan apa-apa

## Deliver Order
```bash
    else if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
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
    }
```
Mengubah status order menjadi delivered jika : 
- nama order sesuai
- Status masih pending (delivered == 0)
- Jenis paket adalah paket yang reguler
- Mengisi nama agen dengan username yang menjalankan program
- Mencatat ke log file
- Menampilkan pesan sukses

### Cleanup
```bash
    else {
        printf("Invalid command.\n");
    }

    shmdt(orders);
    return 0;
}
```
Sebagai error handling dan keluar dari program

### delivery_agent.c

```bash
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

```
### write_log()
Menulis log pengiriman ke file "delivery.log"
```bash
Format log: [timestamp] [agent] Paket dikirim ke [name] di [address]
```
- Menggunakan waktu sistem saat log dibuat

### agent_thread()
- Fungsi yang dijalankan oleh setiap thread agen (AGENT A, B, C), dan loop mencari pesanan Express yang belum dikirim (delivered == 0)

- Menggunakan mutex untuk proteksi akses ke shared memory

Jika menemukan pesanan yang sesuai:
- Tandai sebagai terkirim

- Catat nama agen yang menangani

- Log tercatat di delivery.log

- Sleep 1 detik 

### read_csv()

- Membaca data pesanan dari file CSV ("delivery_order.csv")

- Format CSV: name,address,type

- Menyimpan data ke array orders dalam shared memory

- Inisialisasi status delivered=0 dan agent="-"

### main()
- Membuat/mengakses shared memory segment dengan key 1234

- Membaca data pesanan dari CSV ke shared memory

- Membuat 3 thread agen (A, B, C)

- Menunggu semua thread selesai (meskipun sebenarnya tidak akan selesai karena loop tak terbatas)

- Melepaskan shared memory


## SOAL 3

Dikerjakan oleh : Angga Firmansyah

The Lost Dungeon 
Suatu pagi, anda menemukan jalan setapak yang ditumbuhi lumut dan hampir tertutup semak. Rasa penasaran membawamu mengikuti jalur itu, hingga akhirnya anda melihatnya: sebuah kastil tua, tertutup akar dan hampir runtuh, tersembunyi di tengah hutan. Gerbangnya terbuka seolah memanggilmu masuk.

Di dalam, anda menemukan pintu batu besar dengan simbol-simbol aneh yang terasa… hidup. Setelah mendorongnya dengan susah payah, anda pun menuruni tangga batu spiral yang dalam dan gelap. Di ujungnya, anda menemukan sebuah dunia baru: dungeon bawah tanah yang sudah tertinggal sangat lama.

Anda tidak tahu bagaimana anda dapat berada di situasi ini, tetapi didorong oleh rasa ingin tahu dan semangat, apa pun yang menunggumu di bawah sana, anda akan melawan. (Author: Fico / purofuro)

Entering the dungeon
dungeon.c akan bekerja sebagai server yang dimana client (player.c) dapat terhubung melalui RPC. dungeon.c akan memproses segala perintah yang dikirim oleh player.c. Lebih dari 1 client dapat mengakses server.

Sightseeing 
Anda melihat disekitar dungeon dan menemukan beberapa hal yang menarik seperti toko senjata dan pintu dengan aura yang cukup seram. Ketika player.c dijalankan, ia akan terhubung ke dungeon.c dan menampilkan sebuah main menu seperti yang dicontohkan di bawah ini (tidak harus mirip, dikreasikan sesuai kreatifitas masing-masing praktikan).


Status Check
Melihat bahwa terdapat sebuah toko senjata, anda mengecek status diri anda dengan harapan anda masih memiliki sisa uang untuk membeli senjata. Jika opsi Show Player Stats dipilih, maka program akan menunjukan Uang yang dimiliki (Jumlah dibebaskan), senjata yang sedang digunakan, Base Damage, dan jumlah musuh yang telah dimusnahkan. 


Weapon Shop
Ternyata anda memiliki sisa uang dan langsung pergi ke toko senjata tersebut untuk membeli senjata. Terdapat 5 pilihan senjata di toko tersebut dan beberapa dari mereka memiliki passive yang unik. Disaat opsi Shop dipilih, program akan menunjukan senjata apa saja yang dapat dibeli beserta harga, damage, dan juga passive (jika ada). List senjata yang ada dan dapat dibeli beserta logic/command untuk membeli senjata tersebut diletakan di code shop.c/shop.h yang nanti akan dipakai oleh dungeon.c.

Notes: praktikan dibebaskan untuk penamaan, harga, damage, dan juga passive dari senjata-senjata yang ada. Yang penting harus terdapat 5 atau lebih senjata dengan minimal 2 senjata yang memiliki passive.

Handy Inventory
Setelah membeli senjata di toko tadi, anda membuka ransel anda untuk memakai senjata tersebut. Jika opsi View Inventory dipilih, program akan menunjukan senjata apa saja yang dimiliki dan dapat dipakai (jika senjata memiliki passive, tunjukan juga passive tersebut).

Lalu apabila opsi Show Player Stats dipilih saat menggunakan weapon maka Base Damage player akan berubah dan jika memiliki passive, maka akan ada status tambahan yaitu Passive.


Enemy Encounter
Anda sekarang sudah siap untuk melewati pintu yang seram tadi, disaat anda memasuki pintu tersebut, anda langsung ditemui oleh sebuah musuh yang bukan sebuah manusia. Dengan tekad yang bulat, anda melawan musuh tersebut. Saat opsi Battle Mode dipilih, program akan menunjukan health-bar musuh serta angka yang menunjukan berapa darah musuh tersebut dan menunggu input dengan opsi attack untuk melakukan sebuah serangan dan juga exit untuk keluar dari Battle Mode. Apabila darah musuh berkurang, maka health-bar musuh akan berkurang juga.

Jika darah musuh sudah 0, maka program akan menunjukan rewards berupa berapa banyak gold yang didapatkan lalu akan muncul musuh lagi.


Other Battle Logic
Health & Rewards
Untuk darah musuh, seberapa banyak darah yang mereka punya dibuat secara random, contoh: 50-200 HP. Lakukan hal yang sama untuk rewards. 

Damage Equation
Untuk damage, gunakan base damage sebagai kerangka awal dan tambahkan rumus damage apapun (dibebaskan, yang pasti perlu random number agar hasil damage bervariasi). Lalu buatlah logic agar setiap serangan memiliki kesempatan untuk Critical yang membuat damage anda 2x lebih besar.


Passive
Jika senjata yang dipakai memiliki Passive setiap kali passive tersebut menyala, maka tunjukan bahwa passive tersebut aktif.


Error Handling
Berikan error handling untuk opsi-opsi yang tidak ada.
Contoh:
