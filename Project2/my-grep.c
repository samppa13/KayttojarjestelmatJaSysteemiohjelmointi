#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funktioiden määrittely.
FILE *open_file(char *filename);
void search_term(char *term, FILE *stream);

int main(int argc, char *argv[]) {
    FILE *file;

    // Tarkistetaan, että vähintään hakutermi on annettu komentoriville.
    if (argc < 2) {
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    }
    // Haetaan hakutermiä standardi syötevirrasta.
    else if (argc == 2) {
        search_term(argv[1], stdin);
    }
    // Haetaan hakutermiä annetuista tiedostoista
    else {
        for (int i = 2; i < argc; i++) {
            file = open_file(argv[i]);
            search_term(argv[1], file);
            fclose(file);
        }
    }
    return 0;
}

// Avataan tiedosto.
FILE *open_file(char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) { // Tiedoston avaaminen epäonnistui.
        printf("my-grep: cannot open file\n");
        exit(1);
    }
    return file;
}

void search_term(char *term, FILE *stream) {
    char *line = NULL;
    size_t len = 0;

    // Luetaan annetusta tiedostovirrasta kaikki rivit yksitellen.
    while (getline(&line, &len, stream) != -1) {
        // Tulostetaan rivi, jos hakutermi löytyy kyseiseltä riviltä.
        if (strstr(line, term)) {
            printf("%s", line);
        }
    }

    // Vapautetaan varattu muisti.
    free(line);
}