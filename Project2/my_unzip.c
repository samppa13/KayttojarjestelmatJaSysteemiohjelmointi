#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *tiedosto;
    int toisto;
    int luettu;
    char merkki;

    // Jos tiedostoja ei anneta, tulostetaan ohjeviesti ja poistutaan virhekoodilla 1
    if (argc < 2) {
        fprintf(stderr, "my-unzip: file1 [file2 ...]\n");
        return 1;
    }

    // Käydään läpi kaikki komentoriviltä saadut tiedostot
    for (int i = 1; i < argc; i++) {
        tiedosto = fopen(argv[i], "rb");
        if (tiedosto == NULL) {
            fprintf(stderr, "my-unzip: cannot open file %s\n", argv[i]);
            exit(1);
        }

        // Luetaan pakattu data 5 tavun lohkoissa: ensin 4 tavua kokonaislukuna ja sitten yksi merkki
        while ((luettu = fread(&toisto, sizeof(int), 1, tiedosto)) == 1) {
            if (fread(&merkki, sizeof(char), 1, tiedosto) != 1) {
                fprintf(stderr, "my-unzip: read error\n");
                exit(1);
            }
            // Tulostetaan merkki käyttäen printf()
            for (int j = 0; j < toisto; j++) {
                if (printf("%c", merkki) < 0) {
                    fprintf(stderr, "my-unzip: write error\n");
                    exit(1);
                }
            }
        }
        fclose(tiedosto);
    }
    return 0;
}