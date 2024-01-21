#include <cstdio>

using namespace std;

void hexdump(unsigned char *buffer, int received) {
    int data = 0;
    while (data < received) {
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%02x ", (int)buffer[data]);
            else
                printf("   ");
            data++;
        }
        data -= 16;
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%c", buffer[data] >= 32 && buffer[data] <= 127 ? buffer[data] : '.');
            else
                printf(" ");
            data++;
        }
        printf("\n");
    }
}
