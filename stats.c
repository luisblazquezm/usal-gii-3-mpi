#include <stdio.h>
#include <stdlib.h>
#include "stats.h"

// Standard terminal VT100 is 80 x 25
#define TERM_WIDTH  79
#define TERM_HEIGHT 20

int print_cipher_key(int founder, key_type *key, int numitems);
char* my_itoa(int i);

int main(void)
{
    key_type keys[72] = {"Da9sd8h7HBpoS", "2374988", 0,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 2,
                         "6gFBDF8dfbdff", "9840947", 4,
                         "nf12daOHfpdf6", "1058401", 1,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 2,
                         "89jf4ln89LJk0", "6516808", 3,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 4,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "nf12daOHfpdf6", "9032941", 3,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 4,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 3,
                         "Da9sd8h7HBpoS", "2374988", 2,
                         "Da9sd8h7HBpoS", "2374988", 0,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 2,
                         "6gFBDF8dfbdff", "9840947", 4,
                         "nf12daOHfpdf6", "1058401", 1,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 2,
                         "89jf4ln89LJk0", "6516808", 3,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 4,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "nf12daOHfpdf6", "9032941", 3,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 4,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 3,
                         "Da9sd8h7HBpoS", "2374988", 2,
                         "Da9sd8h7HBpoS", "2374988", 0,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 2,
                         "6gFBDF8dfbdff", "9840947", 4,
                         "nf12daOHfpdf6", "1058401", 1,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 2,
                         "89jf4ln89LJk0", "6516808", 3,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 4,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "nf12daOHfpdf6", "9032941", 3,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 4,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 3,
                         "Da9sd8h7HBpoS", "2374988", 2,
                         "Da9sd8h7HBpoS", "2374988", 0,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 2,
                         "6gFBDF8dfbdff", "9840947", 4,
                         "nf12daOHfpdf6", "1058401", 1,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 2,
                         "89jf4ln89LJk0", "6516808", 3,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 4,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "nf12daOHfpdf6", "9032941", 3,
                         "Da9sd8h7HBpoS", "2374988", 1,
                         "a1bKHLk67hkkh", "4234247", 3,
                         "89jf4ln89LJk0", "6516808", 4,
                         "6gFBDF8dfbdff", "9840947", 2,
                         "nf12daOHfpdf6", "1058401", 3,
                         "Da9sd8h7HBpoS", "2374988", 2};
    
    print_cipher_key(3, keys, 72);

    return 0;
}

char* my_itoa(int i) {

    char *str = NULL;
    
    if (NULL == (str = malloc(12 * sizeof(char)))) // MAX_INT has 10 digits, plus sign
        return NULL;
        
    snprintf(str, sizeof(str), "%d", i);
    
    return str;
}


int print_cipher_key(int founder, key_type *key, int numitems) {
    
    int i, j;
    int items_left = -1;
    char header_format[] = "| %-20s | %-10s | %-10s |\n\
--------------------------------------------------\n"; // Total length 50
    char row_format[] = "| %-20s | %-10s | %-10s |\n";
    
    if (key == NULL || numitems < 0)
        return 1;
        
    items_left = numitems;
    for (i = 0; i <= numitems / (TERM_HEIGHT - 3); ++i) {
    
        system("clear");
       
        printf(header_format, "CIPHER", "KEY", "FOUND BY");
    
        for (j = numitems - items_left; j < numitems
        &&  j / (TERM_HEIGHT - 3) <= i; ++j) {
            printf(row_format, key[j].cipher, key[j].key, "1"); // Change that 1
            --items_left;
        }
        
        putchar('\n');
        if (items_left) {
            printf("\nPress <ENTER> to continue...");
            getchar();
        } else {
            printf("%50s\n", "-----------------------------");
            printf("%41s: %7s\n", "Number of keys found", my_itoa(numitems));
        }
    }
}
