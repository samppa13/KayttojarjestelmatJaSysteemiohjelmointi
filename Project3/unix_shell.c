#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE 1024
#define ERR_MSG "An error has occurred\n"

// Polkua talletetaan taulukkoon, käytetään oletuskena /bin 
static char* polut[128] = {"/bin", NULL};

// Virheilmoitus tulostetaan aina samalla tavalla 
void nayta_virhe() {
    write(STDERR_FILENO, ERR_MSG, strlen(ERR_MSG));
}

// Poimii komentorivin osiin (jakaa &-merkistä, joka tarkoittaa rinnakkaiskomentoja)
char** pilkoRinnakkaiskomennot(char* syote, int* komentoja) {
    char** lista = malloc(sizeof(char*) * 128);
    if (!lista) {
        nayta_virhe();
        exit(1);
    }
    memset(lista, 0, sizeof(char*) * 128);

    int i = 0;
    char* token = strtok(syote, "&");
    while (token != NULL) {
        lista[i++] = token;
        token = strtok(NULL, "&");
    }
    *komentoja = i;
    return lista;
}

/* Poimii yksittäisen komennon ja sen argumentit, tarkista uudelleenohjauksen
   Palauttaa tiedostonimen, jos uudelleenohjaus > on havaittu. */
char* pilkoKomento(char* komento, char** argumentit) {
    char* outTiedosto = NULL;
    int i = 0;
    int uudOhjOperLaskuri = 0;
    char* token = strtok(komento, " \t\n");
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {

            if(uudOhjOperLaskuri){
                nayta_virhe();
                argumentit[0]  = NULL;
                return NULL;
            }
            uudOhjOperLaskuri++;
            // seuraavaksi piäisi tulla tiedostonimi
            token = strtok(NULL, " \t\n");
            if (!token) {
                nayta_virhe();
                argumentit[0] = NULL;
                return NULL;
            }
            outTiedosto = token;

            //Tarkistetaan ylimääräisten argumenttien varalta
            char* extra = strtok(NULL, " \t\n");
            if (extra != NULL) {
                nayta_virhe();
                argumentit[0] = NULL;
                return NULL;
            }
            break;
        
        } else {
            argumentit[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    argumentit[i] = NULL;
    return outTiedosto;
}

// Suoritetaan sisäänrakennetut komennot (exit, cd, path), mikäli niitä havaitaan
int sisaanrakennetutKomennot(char** argit) {
    if (strcmp(argit[0], "exit") == 0) {
        if (argit[1] != NULL) {
            // exit ei ota argumentteja 
            nayta_virhe();
            return 1;
        }
        exit(0);
    }
    else if (strcmp(argit[0], "cd") == 0) {
        if (!argit[1] || argit[2]) {
            nayta_virhe();
            return 1;
        } else {
            if (chdir(argit[1]) != 0) {
                nayta_virhe();
            } else{
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL){
                    printf("%s\n", cwd);
                }                
                
            }
        }
        
        return 1;
    }
    else if (strcmp(argit[0], "path") == 0) {
        //path komento: korvaa vanhan polun 
        for (int i = 0; i < 128; i++) {
            if (polut[i] != NULL && strcmp(polut[i], "/bin") != 0) {
            free(polut[i]);
            }
            polut[i] = NULL;
        }
        int j = 1;
        int idx = 0;
        while (argit[j] != NULL) {
            polut[idx++] = strdup(argit[j]);
            j++;
        }
        polut[idx] = NULL;
        return 1;
    }
    return 0;
}

/* Suorittaa yhden komennon (ei sisäänrakennettu komento)
   - Etsii suoritettavan binaryn poluista
   - Kutsuu execv */
void suoritaKomento(char** argit, char* redirectTiedosto) {
    if (argit[0] == NULL) {
        return;
    }
    // Uudelleenohjaus: ohjataan stdout ja stderr samaan tiedostoon
    int savedOut = dup(STDOUT_FILENO);
    int savedErr = dup(STDERR_FILENO);
    int fd = -1;
    if (redirectTiedosto) {
        fd = open(redirectTiedosto, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            nayta_virhe();
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    // Yritetään polkuja 
    for (int i = 0; polut[i] != NULL; i++) {
        char polkuYhd[512];
        snprintf(polkuYhd, sizeof(polkuYhd), "%s/%s", polut[i], argit[0]);
        if (access(polkuYhd, X_OK) == 0) {
            execv(polkuYhd, argit);
            // Jos execv palaa, se on virhe 
            nayta_virhe();
            break;
        }
    }
    if (fd >= 0) {
        dup2(savedOut, STDOUT_FILENO);
        dup2(savedErr, STDERR_FILENO);
        close(fd);
    }
    nayta_virhe();
    exit(1);
}

// Käsittelee yhtä komentoriviä; rinnakkaiskomennot erotellaan &:llä.
void kasitteleRivi(char* syote) {
    int lkm = 0;
    char** rinnakkaiset = pilkoRinnakkaiskomennot(syote, &lkm);
    if (lkm == 0) {
        free(rinnakkaiset);
        return;
    }

    pid_t pids[128];
    int pcount = 0;

    for (int i = 0; i < lkm; i++) {
        // Pilkotaan komento
        char* argit[128] = {0};
        char* outFile = pilkoKomento(rinnakkaiset[i], argit);
        if (!argit[0]) {
            continue;
        }
        if (sisaanrakennetutKomennot(argit)) {
            continue;
        }
        // Haarautetaan prosessi 
        pid_t pid = fork();
        if (pid < 0) {
            nayta_virhe();
            free(rinnakkaiset);
            return;
        }
        if (pid == 0) {
            // Lapsiprosessissa suoritetaan komento
            suoritaKomento(argit, outFile);
        } else {
            pids[pcount++] = pid;
        }
    }

    for (int i = 0; i < pcount; i++) {
        waitpid(pids[i], NULL, 0);
    }
    free(rinnakkaiset);
}

int main(int argc, char* argv[]) {
    // Käsitellään batch-tilanne
    if (argc > 2) {
        nayta_virhe();
        exit(1);
    }

    FILE* sisaan = stdin;
    if (argc == 2) {
        sisaan = fopen(argv[1], "r");
        if (!sisaan) {
            nayta_virhe();
            exit(1);
        }
    }

    // Silmukka lukee komentorivejä joko tiedostosta tai stdin:stä
    while (1) {
        if (argc == 1) {
            printf("wish> ");
            fflush(stdout);
        }
        char* rivipuskuri = NULL;
        size_t n = 0;
        ssize_t luettu = getline(&rivipuskuri, &n, sisaan);
        if (luettu == -1) {
            free(rivipuskuri);
            exit(0);
        }
        if (rivipuskuri[luettu-1] == '\n') {
            rivipuskuri[luettu-1] = '\0';
        }
        kasitteleRivi(rivipuskuri);
        free(rivipuskuri);
    }
    return 0; 
}