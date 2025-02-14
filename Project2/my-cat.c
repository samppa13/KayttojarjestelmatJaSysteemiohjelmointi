#include <stdio.h>
#include <stdlib.h>

FILE *open_file(char *filename);
void read_files(FILE *stream);

int main(int argc, char *argv[]) {
    FILE *file;
    if (argc < 2) {
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        file = open_file(argv[i]);
        read_files(file);
        fclose(file);
    }
    return 0;
}

FILE *open_file(char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "my-cat: cannot open file\n");
        exit(1);
    }
    return file;
}

void read_files(FILE *stream) {
    char buffer[1024];
    while (fgets(buffer, 1024, stream) != NULL) {
        printf("%s", buffer);
    }
}