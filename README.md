![image](https://github.com/user-attachments/assets/d0fe248f-d9c7-4baf-8569-a3f662373335)# Sisop-3-2025-IT29

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

## DOKUMENTASI SOAL 2

- Download File CSV dengan wget

![image](https://github.com/user-attachments/assets/dd4b3dda-e10e-4d64-8bbe-4118bd774f5f)

- Order Reguler yang berhasil dikirim dengan -deliver Nama

![image](https://github.com/user-attachments/assets/16cff7e5-0c12-4463-b059-14a3f65d9549)

- Delivery List

![image](https://github.com/user-attachments/assets/3c48c17e-98cc-45cf-a072-0bdd8306f1b5)

- Pengecekan status pengiriman

![image](https://github.com/user-attachments/assets/adb9f213-ff9b-4a45-aedc-b5ff60661118)






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

### a.) Entering the dungeon

dungeon.c akan bekerja sebagai server yang dimana client (player.c) dapat terhubung melalui RPC. dungeon.c akan memproses segala perintah yang dikirim oleh player.c. Lebih dari 1 client dapat mengakses server.

Kode penyelesaiian dungeon.c : 

```bash
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
    
```
- Dokumentasi main menu :
![image](https://github.com/user-attachments/assets/2ff17bbf-9b93-49ab-bf2b-b9b0c3880fcf)

- Multiple Player dapat memasuki Server :

![image](https://github.com/user-attachments/assets/d06cc96f-f268-4aee-8bf7-cfb34731691e)

![image](https://github.com/user-attachments/assets/ac8f5f79-a167-4c22-af85-f08e884d4142)

![image](https://github.com/user-attachments/assets/d453fc73-feb9-485f-9a26-93886f7c2f0f)

### Fungsi Log Activity

```bash
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
```

Code diatas adalah function yang digunakan untuk pembuatan logging activity, Dimana ini adalah fitur tambahan untuk log activity yang dapat dilihat dalam folder dan dapat dibuka.

![image](https://github.com/user-attachments/assets/7f3b4f8c-c479-4475-8bd4-cfc1e642da21)

```bash
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(sock, (struct sockaddr *)&client_addr, &len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    
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
```

Fungsi diatas merupakan fungsi untuk handle client server, menggunakan socket programming. Function ini juga digunakan untuk menghamdle fitur battle yang ada didalam program. Serta terdapat error handling apabila terdapat pemilihan opsi diluar pilihan yang tersedia.

### Fungsi Main

```bash
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

```
Fungsi ini sendiri berfungsi sebagai socket server, alamat port server, bind socket ke port, koneksi masuk, dan loop multiclient untuk menerima player dalam jumlah lebih dari satu atau majemuk

### b.) Sightseeing

Soal ini merujuk kepada fitur main menu dalam program ini :

![image](https://github.com/user-attachments/assets/b55d3d35-2282-41d2-93fc-1e0b6e735722)

- Berikut ini merupakan kode yang digunakan untuk menyelesaikan fitur dalam interface player ke dalam server (Permainan). Konten didalam program ini berisi seperti player stats, inventory, Shop, battle, dan exit

```bash
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "shop.c"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;42m"
#define RESET   "\033[0m"

#define PORT 9000
#define BUFFER_SIZE 1024

int sock;

typedef struct {
    int gold;
    Weapon inventory[10];
    int inv_count;
    Weapon equipped;
    int kill_count;
} Player;

Player player = {.gold = 300, .inv_count = 0, .kill_count = 0};

void connect_to_server();
void connect_and_battle();
void show_stats();
void show_inventory();
void shop_menu();

int main() {
    connect_to_server();

    int choice;
    while (1) {
        printf("\n== THE DUNGEON ==\n");
        printf("1. Your Player Stats\n2. View Your Inventory\n3. THE Weapon Shop\n4. BATTLE Mode\n5. Exit\n>> ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: show_stats(); break;
            case 2: show_inventory(); break;
            case 3: shop_menu(); break;
            case 4: connect_and_battle(); break;
            case 5: close(sock); exit(0);
            default: printf("Invalid option.\n");
        }
    }
}

void connect_to_server() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    printf("\033[1;32mPlayer has joined the server.\033[0m\n");
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

void print_healthbar(int hp, int max_hp) {
    int bar_width = 25;
    int filled = (hp * bar_width) / max_hp;

    printf("Enemy health: [");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled)
            printf(GREEN " " RESET);
        else
            printf(" ");
    }
    printf("] %d/%d HP\n", hp, max_hp);
}

void connect_and_battle() {
    srand(time(0));
    char input[10];

    while (1) {
        send(sock, "BATTLE", strlen("BATTLE"), 0);
        char buffer[BUFFER_SIZE] = {0};
        read(sock, buffer, BUFFER_SIZE);

        int enemy_hp, reward;
        sscanf(buffer, "ENEMY %d %d", &enemy_hp, &reward);
        int max_hp = enemy_hp;

        printf("\nEnemy Appeared!\n");
        print_healthbar(enemy_hp, max_hp);

        while (enemy_hp > 0) {
            printf("Type 'attack' to strike or 'exit' to flee: ");
            scanf("%s", input);

            if (strcmp(input, "attack") == 0) {
                int dmg = player.equipped.damage + (rand() % 5);
                int crit = rand() % 100 < 20 ? 1 : 0;
                if (crit) dmg *= 2;
                enemy_hp -= dmg;
                if (enemy_hp < 0) enemy_hp = 0;

                printf("You dealt " RED "%d damage" RESET " %s\n", dmg, crit ? "and defeated the enemy!" : "");
                if (strlen(player.equipped.passive) > 1 && strcmp(player.equipped.passive, "-") != 0)
                    printf("Passive activated: %s\n", player.equipped.passive);

                if (enemy_hp <= 0) {
                    printf("\n=== \033[1;35mREWARD\033[0m ===\n");
                    printf("You earned \033[1;33m%d gold!\033[0m\n", reward);
                    player.gold += reward;
                    player.kill_count++;
                    break;
                } else {
                    print_healthbar(enemy_hp, max_hp);
                }
            } else if (strcmp(input, "exit") == 0) {
                printf("You fled the battle.\n");
                return;
            } else {
                printf("Unknown command.\n");
            }
        }

        printf("\n=== \033[1;36mNEW ENEMY\033[0m ===\n");
    }
}
```
### Status Check

- Berikut merupakan fungsi yang digunakan untuk check player status, yang menggunakan function void yang diisi dengan printf dan memanggil dari struct data player 
```bash
void show_stats() {
    printf("\n== Player Stats ==\n");
    printf("Gold: %d\n",
player.gold);
    printf("Weapon: %s\n", strlen(player.equipped.name) ? player.equipped.name : "None");
    printf("Base Damage: %d\n", player.equipped.damage);
    if (strlen(player.equipped.passive) && strcmp(player.equipped.passive, "-") != 0)
        printf("Passive: %s\n", player.equipped.passive);
    printf("Enemies Defeated: %d\n", player.kill_count);
}
```
Dokumentasi : 

![image](https://github.com/user-attachments/assets/2c63b6c0-bed3-4ee5-bafe-034dee763dc8)

### d.) Weapon Shop

- Berikut ini adalah function void yang digunakan untuk menampilkan menu shop pada main menu player

```bash
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
```

Dokumentasi : 

![image](https://github.com/user-attachments/assets/2b2ceca2-87b9-4f17-a7ca-ea94d1c20172)

### e.) Handy Inventory

- Fungsi show inventory berikut yang menggunakan fungsi void diisi dengan "printf" untuk menampilkan inventory pemain : 

```bash
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
```

Dokumentasi : 

![image](https://github.com/user-attachments/assets/6b577a4e-b3b2-4435-8a68-a2a991acf837)

![image](https://github.com/user-attachments/assets/d6cd464b-0101-4d70-9663-0fb986d1b3e5)

### f.) enemy encounter

- Berikut adalah function connect_and_battle merupakan fungsi yang berhubung juga kedalam server. Jadi string battle akan terkirim untuk meminta musuh baru pada menu player, dan fungsi read() untuk menerima balasan dari server. Efek pasif serta mekanisme kabur juga tertulis didalam fungsi ini.

```bash
void connect_and_battle() {
    srand(time(0));
    char input[10];

    while (1) {
        send(sock, "BATTLE", strlen("BATTLE"), 0);
        char buffer[BUFFER_SIZE] = {0};
        read(sock, buffer, BUFFER_SIZE);

        int enemy_hp, reward;
        sscanf(buffer, "ENEMY %d %d", &enemy_hp, &reward);
        int max_hp = enemy_hp;

        printf("\nEnemy Appeared!\n");
        print_healthbar(enemy_hp, max_hp);

        while (enemy_hp > 0) {
            printf("Type 'attack' to strike or 'exit' to flee: ");
            scanf("%s", input);

            if (strcmp(input, "attack") == 0) {
                int dmg = player.equipped.damage + (rand() % 5);
                int crit = rand() % 100 < 20 ? 1 : 0;
                if (crit) dmg *= 2;
                enemy_hp -= dmg;
                if (enemy_hp < 0) enemy_hp = 0;

                printf("You dealt " RED "%d damage" RESET " %s\n", dmg, crit ? "and defeated the enemy!" : "");
                if (strlen(player.equipped.passive) > 1 && strcmp(player.equipped.passive, "-") != 0)
                    printf("Passive activated: %s\n", player.equipped.passive);

                if (enemy_hp <= 0) {
                    printf("\n=== \033[1;35mREWARD\033[0m ===\n");
                    printf("You earned \033[1;33m%d gold!\033[0m\n", reward);
                    player.gold += reward;
                    player.kill_count++;
                    break;
                } else {
                    print_healthbar(enemy_hp, max_hp);
                }
            } else if (strcmp(input, "exit") == 0) {
                printf("You fled the battle.\n");
                return;
            } else {
                printf("Unknown command.\n");
            }
        }

        printf("\n=== \033[1;36mNEW ENEMY\033[0m ===\n");
    }
}
```
Dokumentasi : 

Serta dibawah ini mencakup poin g.) Other battle logic dimana pasif dari senjata akan berefek kepada musuh yang dihadapi. Healthbar musuh akan random dari 50-200 HP. Damage equation juga terdapat dalam kode diatas dan terdapat di dokumentasi hasil. 

![image](https://github.com/user-attachments/assets/94d073a5-e301-46b7-a517-91dfa1bc7556)


![image](https://github.com/user-attachments/assets/c3a61d94-12ba-4a76-bb0b-0bd36580be60)

### h). Error handling

Error handling akan selalu muncul ketika memilih diluar opsi dari yang diberikan 

Dokumentasi : 

![image](https://github.com/user-attachments/assets/51ae0b9c-11d4-4fa4-a6c7-1fa399b4dc52)

![image](https://github.com/user-attachments/assets/3a768d70-b349-45e5-8dcb-f55bf8cfa882)

![image](https://github.com/user-attachments/assets/8890128d-1672-4293-a0d4-8afdb5cfe1e2)

![image](https://github.com/user-attachments/assets/77a7aba8-806d-43f3-98af-5859f9b06417)



## SOAL 4

Dikerjakan oleh: Bayu Kurniawan

4.	Di dunia yang sedang kacau ini, muncul seorang hunter lemah bernama Sung Jin Woo😈yang bahkan tidak bisa mengangkat sebuah galon🛢️. Suatu hari, hunter ini tertabrak sebuah truk dan meninggal di tempat. Dia pun marah akan dirinya yang lemah dan berharap mendapat kesempatan kedua. Beruntungnya, Sung Jin Woo pun mengalami reinkarnasi dan sekarang bekerja sebagai seorang admin. Uniknya, pekerjaan ini mempunyai sebuah sistem yang bisa melakukan tracking pada seluruh aktivitas dan keadaan seseorang. Sayangnya, model yang diberikan oleh Bos-nya sudah sangat tua sehingga program tersebut harus dimodifikasi agar tidak ketinggalan zaman, dengan spesifikasi (Author: Rafa / kookoon)

a.	Agar hunter lain tidak bingung, Sung Jin Woo memutuskan untuk membuat dua file, yaitu system.c dan hunter.c. Sung Jin Woo mendapat peringatan bahwa system.c merupakan shared memory utama yang mengelola shared memory hunter-hunter dari hunter.c. Untuk mempermudahkan pekerjaannya, Sung Jin Woo mendapat sebuah clue yang dapat membuat pekerjaannya menjadi lebih mudah dan efisien. NOTE : hunter bisa dijalankan ketika sistem sudah dijalankan.

b.	Untuk memastikan keteraturan sistem, Sung Jin Woo memutuskan untuk membuat fitur registrasi dan login di program hunter. Setiap hunter akan memiliki key unik dan stats awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5). Data hunter disimpan dalam shared memory tersendiri yang terhubung dengan sistem.

```bash
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
```

pada fungsi register_hunter dilakukan pengecekan apakah jumlah hunter melebihi batas, jika tidak maka akan membuat hunter baru dengan stats default serta key unik untuk setiap hunternya. pada fungsi login_hunter dilakukan penelusuran array hunter untuk menemukan username yang sesuai dengan input, kemudian melakukan login ke akun hunter tersebut dengan menggunakan key unik milik hunter tersebut.

![image](https://github.com/user-attachments/assets/2c8df226-1202-4b3b-8a73-550a816a0fe5)

c. Agar dapat memantau perkembangan para hunter dengan mudah, Sung Jin Woo menambahkan fitur di sistem yang dapat menampilkan informasi semua hunter yang terdaftar, termasuk nama hunter, level, exp, atk, hp, def, dan status (banned atau tidak). Ini membuat dia dapat melihat siapa hunter terkuat dan siapa yang mungkin melakukan kecurangan.

```bash
void display_hunters() {
    printf("=== HUNTER INFO ===\n");
    for (int i = 0; i < sys_ptr->num_hunters; i++) {
        struct Hunter h = sys_ptr->hunters[i];
        printf("Name: %s\tLevel: %d\tEXP: %d\tATK: %d\tHP: %d\tDEF: %d\tStatus: %s\n",
            h.username, h.level, h.exp, h.atk, h.hp, h.def, h.banned ? "BANNED" : "Active");
    }
}
```
fungsi tersebut digunakan untuk menampilkan seluruh data hunter yang ada. Hal tersebut dapat dilakukan dengan cara melakukan penelusuran pada array hunter kemudian melakukan print pada semua data data hunter.

![image](https://github.com/user-attachments/assets/6462d690-273c-4e58-9ab0-703986ff9239)

d.	Setelah beberapa hari bekerja, Sung Jin Woo menyadari bahwa para hunter membutuhkan tempat untuk berlatih dan memperoleh pengalaman. Ia memutuskan untuk membuat fitur unik dalam sistem yang dapat menghasilkan dungeon secara random dengan nama, level minimal hunter, dan stat rewards. Setiap dungeon akan disimpan dalam shared memory sendiri yang berbeda dan dapat diakses oleh hunter

```bash
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
```
fungsi diatas digunakan untuk membuat dungeons baru secara random. Pertama fungsi akan mengecek limit jumlah dungeons, kemudian akan menghasilkan nilai minimal lvl, atk, hp, def, serta exp yang didapatkan dari melakukan raid pada dungeon tersebut. (Note: nama nama dungeon pada file 'dungeon' tidak digunakan sehingga nama dungeon tidak dilakukan randomisasi dan akan selalu memiliki nama "Demon Castle"). Setelah itu juga membuat key random yang akan dilakukan pointer pada system.

![image](https://github.com/user-attachments/assets/ddfb6e07-fdbd-4ccd-8bd4-eeaa2553b6bd)

e.	Untuk memudahkan admin dalam memantau dungeon yang muncul, Sung Jin Woo menambahkan fitur yang menampilkan informasi detail semua dungeon. Fitur ini menampilkan daftar lengkap dungeon beserta nama, level minimum, reward (EXP, ATK, HP, DEF), dan key unik untuk masing-masing dungeon.

```bash
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
```

Fungsi ini melakukan perulangan untuk menampilkan setiap index pada array dungeon beserta dengan data-data yang ada didalamnya seperti nama, minimum lvl, atk, hp, def, dan exp yang didapatkan, serta key uniknya

![image](https://github.com/user-attachments/assets/f950dc8c-0259-441f-8a1e-96ae78e07fc3)

f.	Pada saat yang sama, dungeon yang dibuat oleh sistem juga harus dapat diakses oleh hunter. Sung Jin Woo menambahkan fitur yang menampilkan semua dungeon yang tersedia sesuai dengan level hunter. Disini, hunter hanya dapat menampilkan dungeon dengan level minimum yang sesuai dengan level mereka

```bash
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
```

fungsi tersebut berguna untuk mengakses data sistem berupa dungeon mana saja yang dapat di raid. Hal tersebut dilakukan dengan mengakses data sistem kemudian dilakukan pengecekan terhadap lvl minimun dengan lvl hunter, jika kondisi terpenuhi maka dungeon akan ditampilkan, jika tidak maka tidak ditampilkan pada list.

![image](https://github.com/user-attachments/assets/a0ff6299-6a1d-4129-86d5-bfea178bf33e)

g.	Setelah melihat beberapa hunter yang terlalu kuat, Sung Jin Woo memutuskan untuk menambahkan fitur untuk menguasai dungeon. Ketika hunter berhasil menaklukan sebuah dungeon, dungeon tersebut akan menghilang dari sistem dan hunter akan mendapatkan stat rewards dari dungeon. Jika exp hunter mencapai 500, mereka akan naik level dan exp kembali ke 0.

```bash
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
```

fungsi ini digunakan untuk melakukan raid pada dungeon tertentu. pertama fungsi akan menampilkan list dungeons yang dapat di raid, kemudian fungsi akan meminta input dungeon mana yang akan di raid, setelah itu hunter akan mendapatkan reward hasil dari raid dungeon tersebut. setelah itu fungsi juga akan mengecek exp hunter, jika melebihi 500 maka level hunter akan bertambah 1. setelah itu akan dilakukan shifting pada array dungeon serta menghapus dungeon yang telah di raid.

![image](https://github.com/user-attachments/assets/caf9e1e3-a43a-4dd4-bbe2-959f7f40d223)

h.	Karena persaingan antar hunter semakin ketat, Sung Jin Woo mengimplementasikan fitur dimana hunter dapat memilih untuk bertarung dengan hunter lain. Tingkat kekuatan seorang hunter bisa dihitung melalui total stats yang dimiliki hunter tersebut (ATK+HP+DEF). Jika hunter menang, maka hunter tersebut akan mendapatkan semua stats milik lawan dan lawannya akan terhapus dari sistem. Jika kalah, hunter tersebutlah yang akan dihapus dari sistem dan semua statsnya akan diberikan kepada hunter yang dilawannya.

```bash
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
```

fungsi ini digunakan untuk melakukan duel antar hunter, dimana hunter yang kalah akan dihapus dari system beserta dengan shared memorynya. Pertama akan ditampilkan list hunter lain untuk diajak duel, hunter akan memilih 1 lawan untuk melakukan duel. selanjutnya akan dibandingkan power kedua hunter yang dimana power dihasilkan dari penjumlahan antara ATK, HP, dan DEF. Hunter yang menang akan mendapatkan ATK, HP, dan DEF milik lawannya.

![image](https://github.com/user-attachments/assets/be17bca1-823b-4f35-be16-4741584b9c7c)
![image](https://github.com/user-attachments/assets/ecb4de58-54e4-4f9f-94a0-2809a0bf6600)
