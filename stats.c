#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stats.h"

// Standard terminal VT100 is 80 x 25
#define TERM_WIDTH  79
#define TERM_HEIGHT 20
#define ITEMS_PER_PAGE 10

int print_table(char **colname, char **item, int nrows, int ncols);
int print_cipher_key(int founder, key_type *key, int numitems);
char* my_itoa(int i);
int format_table(char **colname,   // Column headings
                 char **item,     // Table content
                 int nrows,        // Number of rows
                 int ncols,        // Number of columns
                 char *heading,    // Buffer for heading string
                 char *row_format, // Buffer for row format
                 int max_width,    // 'heading''s size
                 int format_size);

int main(void)
{
    char *titles[3] = {"CIPHER", "KEY", "FOUNDER"};
    char* keys[75] = {"Da9sd8h7HBpoS", "2374988", "0",
                         "a1bKHLk67hkkh", "4234247", "3",
                         "89jf4ln89LJk0", "6516808", "2",
                         "6gFBDF8dfbdff", "9840947", "4",
                         "nf12daOHfpdf6", "1058401", "1",
                         "Da9sd8h7HBpoS", "2374988", "1",
                         "a1bKHLk67hkkh", "4234247", "2",
                         "a1bKHLk67hkkh", "4234247", "3",
                         "89jf4ln89LJk0", "6516808", "2",
                         "6gFBDF8dfbdff", "9840947", "4",
                         "nf12daOHfpdf6", "1058401", "1",
                         "Da9sd8h7HBpoS", "2374988", "1",
                         "a1bKHLk67hkkh", "4234247", "2",
                         "a1bKHLk67hkkh", "4234247", "3",
                         "89jf4ln89LJk0", "6516808", "2",
                         "6gFBDF8dfbdff", "9840947", "4",
                         "nf12daOHfpdf6", "1058401", "1",
                         "Da9sd8h7HBpoS", "2374988", "1",
                         "a1bKHLk67hkkh", "4234247", "2",
                         "a1bKHLk67hkkh", "4234247", "3",
                         "89jf4ln89LJk0", "6516808", "2",
                         "6gFBDF8dfbdff", "9840947", "4",
                         "nf12daOHfpdf6", "1058401", "1",
                         "Da9sd8h7HBpoS", "2374988", "1",
                         "a1bKHLk67hkkh", "4234247", "2"};
    
    print_table(titles, keys, 25, 3);

    return 0;
}

char* my_itoa(int i) {

    char *str = NULL;
    
    if (NULL == (str = malloc(12 * sizeof(char)))) // MAX_INT has 10 digits, plus sign
        return NULL;
        
    snprintf(str, sizeof(str), "%d", i);
    
    return str;
}

int print_table(char **colname, char **item, int nrows, int ncols) {
    
    int i, j;
    int rows_left = -1;
    char heading[TERM_WIDTH];
    char row_format[100];
    
    if (colname == NULL || item == NULL || nrows < 0 || ncols < 0,
    
        /* This limitation is crap, but I have to think of it */ ncols > 4){
        return 1;
    }
        
    format_table(colname,
                 item,
                 nrows,
                 ncols,
                 heading,
                 row_format,
                 TERM_WIDTH,
                 100);
        
    rows_left = nrows;
    for (i = 0; i <= nrows / (ITEMS_PER_PAGE - 3); ++i) {
    
        system("clear");
        
        printf("%s\n", heading);
        for (j = 0; j < strlen(heading); ++j)
            putchar('=');
        putchar('\n');
        
        for (j = nrows - rows_left;
             j < nrows && j / (ITEMS_PER_PAGE - 3) <= i;
             ++j)
        {
            // Crap reflexes here <-----------------------------------------------------------
            switch(ncols) {
                case 1:
                    printf(row_format, *(item +j*ncols + 0));
                    break;
                case 2:
                    printf(row_format, *(item +j*ncols + 0),
                                       *(item +j*ncols + 1));
                    break;
                case 3:
                    printf(row_format, *(item +j*ncols + 0),
                                       *(item +j*ncols + 1),
                                       *(item +j*ncols + 2));
                    break;
                case 4:
                    printf(row_format, *(item +j*ncols + 0),
                                       *(item +j*ncols + 1),
                                       *(item +j*ncols + 2),
                                       *(item +j*ncols + 3));
                    break;
            }
            --rows_left;
        
        }
        
        putchar('\n');
        if (rows_left) {
            printf("\nPress <ENTER> to continue...");
            getchar();
        } else {
            printf("%s\n", "-----------------------------");
            printf("%s: %s\n\n\n", "Number of keys found", my_itoa(nrows * ncols));
        }
    }
    
    return 0;
}

int format_table(char **colname,   // Column headings
                 char **item,     // Table content
                 int nrows,        // Number of rows
                 int ncols,        // Number of columns
                 char *heading,    // Buffer for heading string
                 char *row_format, // Buffer for row format
                 int max_width,    // 'heading''s size
                 int format_size)  // 'row_format''s size
{
    int i, j;
    int tmp;
    int *col_width;
    int available_width = TERM_WIDTH;
    char buf[50], buf1[10];
    
    if ((col_width = calloc(ncols, sizeof(int))) == NULL) // Initialized to 0
        return 1;
    
    /* For each column we must find its width
     * 
     */
    for (i = 0; i < ncols; ++i) {

        col_width[i] = strlen(colname[i]);
        for (j = 0; j < nrows; ++j) {
            tmp = strlen(*(item + j*ncols + i));
            if (tmp > col_width[i]) {
                col_width[i] = tmp;
            }
        }
        
    }    
    
    // If columns' width does not cover terminal's width, distribute the rest of
    // the width evenly among the columns
    for (i = 0; i < ncols; ++i){
        available_width -= col_width[i];
    }
    available_width -= (3*ncols + 1); // Because the format is going to be | HEADING1 | HEADING2 | ... | HEADINGN |
                                      //                                   ^^        ^^^        ^      ^^        ^^
                                      // there are three extra characters in each title: vertical bar, space, space
                                      // plus one because of the last vertical bar
    if (available_width > 0) {
        
        tmp = available_width / ncols;
        
        for (i = 0; i < ncols; ++i)
            col_width[i] += tmp;   
        
    }
    
    // EVERYTHING OKAY UNTIL HERE
    
    // Now build heading...
    snprintf(heading, max_width, "| %-*s ", col_width[0], colname[0]);
    for (i = 1; i < ncols; ++i){
        snprintf(buf, sizeof(buf), "| %-*s ", col_width[i], colname[i]);
        strncat(heading, buf,  sizeof(heading) - strlen(heading));
    }
    strncat(heading, "|", sizeof(heading) - strlen(heading));
    
    //... and row format;
    for (i = 0; i < ncols; ++i){
    
        snprintf(buf, sizeof(buf), "%s", "| %-");
        snprintf(buf1, sizeof(buf1), "%d", col_width[i]);
        strncat(buf, buf1, sizeof(buf) - strlen(buf));
        strncat(buf, "s ", sizeof(buf) - strlen(buf));
        
        if (i == 0) {
            snprintf(row_format, format_size,  "%s", buf);
        } else {
            strncat(row_format, buf, format_size - strlen(row_format));
        }
    }
    strncat(row_format, "|\n", format_size - strlen(row_format));
    
    free(col_width);
    
    return 0;
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

	return 0;
}
