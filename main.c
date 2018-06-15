#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF 50

struct TOWAR {

    int kod_kreskowy;
    char nazwa[MAX_BUF];
    double cena;
};


int main(int argc, char *argv[])
{
    struct TOWAR towary[MAX_BUF];
    
    FILE * fp;
    char * line = NULL;
    char * token = NULL;
    size_t len = 0;
    ssize_t read;
    int i = 0;

    fp = fopen("test.txt", "r+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if ((token = strtok(line, " ")) == NULL)
            exit(EXIT_FAILURE);
        while (token != NULL)
        {
            sscanf(token, "%d", &towary[i].kod_kreskowy);
            token = strtok (NULL, " ");
            
            sscanf(token, "%s", towary[i].nazwa);
            token = strtok (NULL, " ");
            
            sscanf(token, "%lg", &towary[i].cena);
            token = strtok (NULL, " ");
        }
        i++;
    }

    fclose(fp);
    if (line)
        free(line);
    
    int szukany_kod_kreskowy, znaleziono = 0;
    if (argc >= 3)
    {
        sscanf(argv[2], "%d", &szukany_kod_kreskowy);
        printf("[-]Szukany kod kreskowy: %d\n", szukany_kod_kreskowy);
    }
    
    if (strcmp(argv[1], "skasuj") == 0)
    {
        printf("[+]Tryb kasowania.\n");
        
            for (int j = 0; j < i; j++)
            {
                if (towary[j].kod_kreskowy == szukany_kod_kreskowy)
                {
                    printf("[+]Znaleziono!\n");
                    strncpy(towary[j].nazwa, "kod_kreskowy_do_skasowania", sizeof(towary[j].nazwa));
                    znaleziono = 1;
                    break;
                }
            }
        
        if (znaleziono == 0)
        {
            printf("[-]Nie znaleziono towaru o zadanym ID!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    else if (strcmp(argv[1], "zmien") == 0)
    {
        printf("[+]Tryb zmiany nazwy lub ceny.\n");
        for (int j = 0; j < i; j++)
        {
            if (towary[j].kod_kreskowy == szukany_kod_kreskowy)
            {
                printf("[+]Znaleziono!\n");
                sscanf(argv[4], "%lf", &towary[j].cena);
                strncpy(towary[j].nazwa, argv[3], sizeof(towary[j].nazwa));
                znaleziono = 1;
                break;
            }
        }
        
        if (znaleziono == 0)
        {
            printf("[-]Nie znaleziono towaru o zadanym ID!\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(argv[1], "dodaj") == 0)
    {
        printf("[+]Tryb dodawania produktu pod indeksem: %d.\n", i);

        towary[i].kod_kreskowy = szukany_kod_kreskowy;
        sscanf(argv[4], "%lf", &towary[i].cena);
        strncpy(towary[i].nazwa, argv[3], sizeof(towary[i].nazwa));
        i++;
        printf("[+]Dodano produkt o zadanym ID!\n");
    }
    else if (strcmp(argv[1], "pokaz_all") == 0)
    {
        printf("[+]Tryb listy produktów.\n");
        for (int j = 0; j < i; j++)
        {
            printf ("%d %s %.2f\n", towary[j].kod_kreskowy, towary[j].nazwa, towary[j].cena);
        }
        
    }
    else if (strcmp(argv[1], "pokaz") == 0)
    {
        printf("[+]Tryb pokazywania pojedynczego produktu.\n");
        for (int j = 0; j < i; j++)
        {
            if (towary[j].kod_kreskowy == szukany_kod_kreskowy)
            {
                printf("[+]Znaleziono!\n");
                printf ("%d %s %.2f\n", towary[j].kod_kreskowy, towary[j].nazwa, towary[j].cena);
                znaleziono = 1;
                break;
            }
        }
        
        if (znaleziono == 0)
        {
            printf("[-]Nie znaleziono towaru o zadanym ID!\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("[+]Tryb nieokreślony.\n");
        exit(EXIT_FAILURE);
    }
    
    
    /*zapisywanie bazy ponownie do tego samego pliku*/
    fp = fopen("test.txt", "wb");
    for (int j = 0; j < i; j++)
    {
        if (strcmp(towary[j].nazwa, "kod_kreskowy_do_skasowania") == 0) continue;
        fprintf (fp, "%d %s %.2f\n", towary[j].kod_kreskowy, towary[j].nazwa, towary[j].cena);
    }
    fclose(fp);
    
    exit(EXIT_SUCCESS);
}
