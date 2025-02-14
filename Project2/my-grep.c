#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *open_file(char *filename);
void search_term(char *term, FILE *stream);

int main(int argc, char *argv[]) {
    FILE *file;
    if (argc < 2) {
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    }
    else if (argc == 2) {
        search_term(argv[1], stdin);
    }
    else {
        for (int i = 2; i < argc; i++) {
            file = open_file(argv[i]);
            search_term(argv[1], file);
            fclose(file);
        }
    }
    return 0;
}

FILE *open_file(char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        printf("my-grep: cannot open file\n");
        exit(1);
    }
    return file;
}

void search_term(char *term, FILE *stream) {
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, stream) != -1) {
        if (strstr(line, term)) {
            printf("%s", line);
        }
    }
    free(line);
}