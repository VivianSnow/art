#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include "art.h"

#define COPYINCR (1024*1024*32)


int *mem_start;
char s[120];
art_tree t;

int art_from_file_mmap(char* filename)
{
    size_t searchsz;
    size_t copysz;
    off_t fsz = 0;
    char *src;
    char *s_remain = NULL;
    struct stat sbuf;
    char *point;
    int fdin = open(filename, O_RDONLY);
    fstat(fdin, &sbuf);

    int res = art_tree_init(&t);
    if(res != 0) return -1;

    while (fsz < sbuf.st_size) {
        if ((sbuf.st_size - fsz) > COPYINCR)
            copysz = COPYINCR;
        else
            copysz = sbuf.st_size - fsz;
        src = (char *) mmap(0, copysz, PROT_READ, MAP_SHARED, fdin, fsz);
        searchsz = copysz;

        while (point = memchr(src, '\n', searchsz)) {
            memcpy(s, src, point - src);
            searchsz -= (point - src + 1);
            s[point - src] = '\0';
            src = point + 1;
            if (s_remain) {
                s_remain = strcat(s_remain, s);
                int len = strlen(s_remain);
                art_insert(&t, (unsigned char*)s_remain, len, "1");
                free(s_remain);
                s_remain = NULL;
            }
            else{
                int len = strlen(s);
                art_insert(&t, (unsigned char*)s, len, "1");
            }
        }
        if (copysz == COPYINCR) {
            s_remain = (char *) malloc(sizeof(char)*105);
            s_remain = memcpy(s_remain, src, searchsz);
            s_remain[searchsz] = '\0';
        }
        munmap(src, copysz);
        fsz += copysz;
    }
}

void check_from_file(char *filename)
{
    FILE *fp;
    char StrLine[200];
    if((fp = fopen(filename, "r")) == NULL)
    {
        return;
    }
    FILE *fout;
    fout = fopen("/home/viviansnow/data/art_out.txt", "w");
    while(!feof(fp))
    {
        fgets(StrLine, 200, fp);
        StrLine[strlen(StrLine) - 2] = '\0';
        int len = strlen(StrLine);
        if(art_search(&t, (unsigned char*)StrLine, len))
        {
            fputs("True\n",fout);
        }
        else
        {
            fputs("False\n", fout);
        }
    }
    fclose(fout);
}

int main()
{
    art_from_file_mmap("/home/viviansnow/data/data2.txt");
    check_from_file("/home/viviansnow/data/checklist.dat");
    return 0;
}
