#include <stdio.h>
#include <stdlib.h>

// Funktioiden määrittely.
FILE *open_file(char *filename);
void read_files(FILE *stream);

int main(int argc, char *argv[]) {
    FILE *file;

    // Tarkistetaan, että vähintään yksi tiedosto on annettu komentorivillä.
    if (argc < 2) {
        return 0;
    }

    // Käydään läpi jokainen tiedosto yksitellen.
    for (int i = 1; i < argc; i++) {
        file = open_file(argv[i]);
        read_files(file);
        fclose(file);
    }
    return 0;
}

FILE *open_file(char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) { // Tiedoston avaaminen epäonnistui.
        fprintf(stderr, "my-cat: cannot open file\n");
        exit(1);
    }
    return file;
}

void read_files(FILE *stream) {
    char buffer[1024];
    // Luetaan kaikki rivit tiedostosta ja tulostetaan ne komentoriville.
    while (fgets(buffer, 1024, stream) != NULL) {
        printf("%s", buffer);
    }
}