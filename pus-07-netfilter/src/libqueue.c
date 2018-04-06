#include <stdio.h>

unsigned short internet_checksum(unsigned short *addr, int count) {

    register int sum = 0;

    while (count > 1) {
        sum += *addr++;
        count -= 2;
    }

    if (count > 0) {
        sum +=   *(unsigned char *) addr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (~sum);
}


unsigned char* swap_bytes(unsigned char *data, unsigned int data_length) {

    unsigned char tmp;
    unsigned char *head, *tail;

    head = data;
    tail = &data[data_length - 1];

    while (head < tail) {
        tmp = *head;
        *head = *tail;
        *tail = tmp;
        head++;
        tail--;
    }

    return data;
}
