#ifndef __STATS_H
#define __STATS_H

#include "crypting.h"

// Standard terminal VT100 is 80 x 25
#define TERM_WIDTH      79
#define TERM_HEIGHT     24
#define ITEMS_PER_PAGE  22
#define N_PROCS         5

// Not number pi. 3 is the number of columns and 14 is the widest's cell size
#define N_COLS_KEY_TABLE    3
#define N_COLS_PROC_TABLE    2
#define WIDEST_CELL_WIDTH_KEY_TABLE   14
#define WIDEST_CELL_WIDTH_PROC_TABLE  5

char** str_split(char* a_str, const char a_delim);

int print_table(char *colname,
                char *item,
                int nrows,
                int ncols,
                char *last_page_message);

char* my_itoa(int i);

int format_table(char **colname,
                 char **item,
                 int nrows,
                 int ncols,
                 char *heading,
                 char *row_format,
                 int max_width,
                 int format_size);
      
                 
int display_stats(char **found_keys, int nkeys,
                  char **keys_per_proc, int nprocs,
                  int n_calls_rand_crypt,
                  int nattempts,
                  double secs_per_key,
                  double runtime);


#endif
