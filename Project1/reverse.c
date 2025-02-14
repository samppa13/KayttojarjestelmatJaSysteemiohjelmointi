#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct lines {
    char *string;
    struct lines *prev;
    struct lines *next;
} Lines;

FILE *open_file(char *filename, char *mode);
Lines *read_lines(Lines *line, FILE *stream);
void print_lines(Lines *line, FILE *stream);
void delete_lines(Lines *line);

int main(int argc, char *argv[]) {
    Lines *line = NULL;
    FILE *file;
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
    else if (argc == 1) {
        line = read_lines(line, stdin);
        print_lines(line, stdout);
        delete_lines(line);
    }
    else if (argc == 2) {
        file = open_file(argv[1], "r");
        line = read_lines(line, file);
        fclose(file);
        print_lines(line, stdout);
        delete_lines(line);
    }
    else if (argc == 3) {
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "Input and output file must differ\n");
            exit(1);
        }
        file = open_file(argv[1], "r");
        line = read_lines(line, file);
        fclose(file);
        file = open_file(argv[2], "w");
        print_lines(line, file);
        fclose(file);
        delete_lines(line);
    }
    return 0;
}

FILE *open_file(char *filename, char *mode) {
    FILE *file;
    if ((file = fopen(filename, mode)) == NULL) {
        fprintf(stderr, "error: cannot open file '%s'\n", filename);
        exit(1);
    }
    return file;
}

Lines *read_lines(Lines *line, FILE *stream) {
    char *ptrLine = NULL;
    size_t len = 0;
    Lines *newLine;
    while (getline(&ptrLine, &len, stream) != -1) {
        if ((newLine = malloc(sizeof(Lines))) == NULL) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        if ((newLine->string = malloc(strlen(ptrLine) + 1)) == NULL) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        strcpy(newLine->string, ptrLine);
        newLine->prev = newLine->next = NULL;
        if (line == NULL) {
            line = newLine;
        }
        else {
            line->next = newLine;
            newLine->prev = line;
            line = newLine;
        }
    }
    free(ptrLine);
    return line;
}

void print_lines(Lines *line, FILE *stream) {
    Lines *ptr = line;
    while (ptr != NULL) {
        fprintf(stream, "%s", ptr->string);
        ptr = ptr->prev;
    }
}

void delete_lines(Lines *line) {
    Lines *ptr = line;
    while (ptr != NULL) {
        line = ptr->prev;
        free(ptr->string);
        free(ptr);
        ptr = line;
    }
}