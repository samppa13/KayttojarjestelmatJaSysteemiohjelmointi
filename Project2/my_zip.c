#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *tiedosto;
    int lukumaara;
    int merkki, edellinen;

    // Tarkistetaan, että tiedostopolkuja on annettu komentorivillä
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file [file...]\n", argv[0]);
        return 1;
    }

    // Käydään läpi jokainen tiedosto yksitellen ja tehdään virheentarkistus
    for (int i = 1; i < argc; i++) {
        tiedosto = fopen(argv[i], "r");
        if (tiedosto == NULL) {
            fprintf(stderr, "my-zip: cannot open file %s\n", argv[i]);
            exit(1);
        }

        // Luetaan ensimmäinen merkki tiedostosta
        merkki = fgetc(tiedosto);
        if (merkki == EOF) {
            fclose(tiedosto);
            continue; // Jatketaan suoritusta jos on tyhjä
        }
        edellinen = merkki;
        lukumaara = 1;

        // Luetaan akikki rivit tiedostosta
        while ((merkki = fgetc(tiedosto)) != EOF) {
            if (merkki == edellinen) {
                lukumaara++; 
            } else {
                // Kirjoitetaan ensin 4 tavua sisältävä integer ja sen jälkeen yksi ASCII-merkki
                if (fwrite(&lukumaara, sizeof(int), 1, stdout) != 1) {
                    fprintf(stderr, "my-zip: write error\n");
                    exit(1);
                }
                if (fputc(edellinen, stdout) == EOF) {
                    fprintf(stderr, "my-zip: write error\n");
                    exit(1);
                }
                // Vaihdetaan seuraavaan merkkiin ja nollataan laskuri
                edellinen = merkki;
                lukumaara = 1;
            }
        }

        // Kirjoitetaan viimeinen merkki ja siihen liittyvä määrä.
        if (fwrite(&lukumaara, sizeof(int), 1, stdout) != 1) {
            fprintf(stderr, "my-zip: write error\n");
            exit(1);
        }
        if (fputc(edellinen, stdout) == EOF) {
            fprintf(stderr, "my-zip: write error\n");
            exit(1);
        }

        fclose(tiedosto);
    }
    return 0;
}