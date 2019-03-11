#ifndef __STATS_H
#define __STATS_H

typedef struct key_type {
    char *cipher;
    char *key;
    int key_id;
} key_type;

/* I/O functions */
int print_cipher_key(int founder, key_type *key, int numitems);
char* my_itoa(int i);


#endif
