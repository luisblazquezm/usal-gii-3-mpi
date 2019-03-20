#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stats.h"

/*
int main(void)
{
    char* keys[25][3] = {"Da9sd8h7HBpoS", "2374988", "0",
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
                         
    char *keys_per_proc[5][2] = {"0", "6",
                                 "1", "5",
                                 "2", "3",
                                 "3", "5",
                                 "4", "6"};
    
    display_stats((char **)keys, 25,
                  (char **)keys_per_proc, 5,
                  23423,
                  2342,
                  12.9,
                  1234.23);

    return 0;
}
*/

void print_horizontal_bar(char c, int len, int newline)
{
    int i;
    
    for (i = 0; i < len; ++i)
        putchar(c);
    if (newline)
        putchar('\n');
}

int display_stats(char **found_keys, int nkeys,
                  char **keys_per_proc, int nprocs,
                  int n_calls_rand_crypt,
                  int nattempts,
                  double secs_per_key,
                  double runtime)
{
    int i, tmp;

    char border_char = '*';
    char inner_char = '-';
    char *titles_keys[3] = {"TEXTO CIFRADO", "CLAVE", "DESCUBRIDOR",};
    char *titles_keys_per_proc[2] = {"PROCESO", "NUMERO DE CLAVES"};
    char sumup_title[] = "RESUMEN";
    char buf[100];
    int padding = 8;
    
    // Display tables
        // Display found keys
        // Display keys per process
    
    // Display non-tables
        // Display calls to rand() and crypt()
        // Display number of attempts
        // Display mean seconds per key
        // Display number of processes
        // Display running time
        
    // DISPLAY TABLES
    snprintf(buf, sizeof(buf), "Numero de claves encontradas: %d", nkeys);

    print_table((char *)titles_keys, (char *)found_keys, nkeys, 3, buf);

printf("-------------------------------------------------> 1\n"); getchar(); // DEBUG   

    print_table((char *)titles_keys_per_proc, (char *)keys_per_proc, nprocs, 2, NULL);

printf("-------------------------------------------------> 2\n"); getchar(); // DEBUG   
    
    // DISPLAY NON-TABLES
    system("clear");
    
    print_horizontal_bar(border_char, TERM_WIDTH, 1);
    tmp = (TERM_WIDTH - strlen(sumup_title))/2 - 2;
    printf ("%c %*s%-s%*s %c\n", border_char, tmp, "", sumup_title, tmp, "", border_char);
    
    print_horizontal_bar(border_char, TERM_WIDTH, 1);

    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    snprintf(buf, sizeof(buf), "Numero de procesos: %d procesos", nprocs);
    printf ("%c %-*s %c\n", border_char, TERM_WIDTH - 4, buf, border_char);
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    putchar(border_char);
    print_horizontal_bar(inner_char, TERM_WIDTH - 2, 0);
    putchar(border_char);
    putchar('\n');
    
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    snprintf(buf, sizeof(buf), "Estadisticas");
    printf ("%c %-*s %c\n", border_char, TERM_WIDTH - 4, buf, border_char);
    
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    
    snprintf(buf, sizeof(buf), "Numero de llamadas a rand/crypt: %d llamadas", n_calls_rand_crypt);
    printf ("%-*c- %-*s %c\n", padding + 1, border_char, TERM_WIDTH - padding - 5, buf, border_char);
    
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    snprintf(buf, sizeof(buf), "Numero de intentos: %d intentos", nattempts);
    printf ("%-*c- %-*s %c\n", padding + 1, border_char, TERM_WIDTH - padding - 5, buf, border_char);
    
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    snprintf(buf, sizeof(buf), "Numero medio de segundos por clave: %.4lf s", secs_per_key);
    printf ("%-*c- %-*s %c\n", padding + 1, border_char, TERM_WIDTH - padding - 5, buf, border_char);
    
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    
    putchar(border_char);
    print_horizontal_bar(inner_char, TERM_WIDTH - 2, 0);
    putchar(border_char);
    putchar('\n');
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    snprintf(buf, sizeof(buf), "Tiempo total de ejecucion: %.4lf s", runtime);
    printf ("%c %-*s %c\n", border_char, TERM_WIDTH - 4, buf, border_char);
    printf("%c%*s%c\n", border_char, TERM_WIDTH-2, "", border_char);
    print_horizontal_bar(border_char, TERM_WIDTH, 1);
    
    printf("\nPress <ENTER> to continue...");
    getchar();
    
    return 0;
    
}

char* my_itoa(int i) {

    char *str = NULL;
    
    if (NULL == (str = malloc(12 * sizeof(char)))) // MAX_INT has 10 digits, plus sign
        return NULL;
        
    snprintf(str, sizeof(str), "%d", i);
    
    return str;
}

int print_table(char *_colname,
                char *_item,
                int nrows,
                int ncols,
                char *last_page_message) {
    
    int i, j;
    int rows_left = -1;
    char heading[TERM_WIDTH];
    char row_format[100];
    char **colname;
    char **item;
    
    if (colname == NULL || item == NULL || nrows < 0 || ncols < 0,
    
        /* This limitation is crap, but I have to think of it */ ncols > 4){
        return 1;
    }
    
    colname = str_split(_colname, '\0');
    item = str_split(_item, '\0');

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
            // Crap has this consequence <-----------------------------------------------------------
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
            if (last_page_message != NULL) {
                print_horizontal_bar('-', strlen(last_page_message), 1);
                printf("%s\n\n\n", last_page_message);
            }
            printf("\nPress <ENTER> to continue...");
            getchar();
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

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/*
int main()
{
    char months[] = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC";
    char** tokens;

    printf("months=[%s]\n\n", months);

    tokens = str_split(months, ',');

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            printf("month=[%s]\n", *(tokens + i));
            free(*(tokens + i));
        }
        printf("\n");
        free(tokens);
    }

    return 0;
}
*/

/*
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
*/
