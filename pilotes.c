#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/pilotes.h"
#include "lib/voitures.h"

// Initialiser les pilotes à partir d'un fichier CSV
void initPilotes(Pilote pilotes[], const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int i = 0;

    // Lire chaque ligne du fichier CSV
    while (fgets(line, sizeof(line), file) && i < 20) {
        // Suppression du saut de ligne
        line[strcspn(line, "\n")] = '\0';

        // Découper la ligne en utilisant la virgule comme séparateur
        char* token = strtok(line, ",");
        pilotes[i].first_name = strdup(token);

        token = strtok(NULL, ",");
        pilotes[i].last_name = strdup(token);

        token = strtok(NULL, ",");
        pilotes[i].number = atoi(token);

        token = strtok(NULL, ",");
        pilotes[i].team_name = strdup(token);

        token = strtok(NULL, ",");
        pilotes[i].nickname = strdup(token);

        token = strtok(NULL, ",");
        pilotes[i].points = atoi(token);

        i++;
    }

    fclose(file);
}

// Afficher les informations des pilotes
void afficherPilotes(Pilote pilotes[], int taille) {
    for (int i = 0; i < taille; i++) {
        printf("Pilote %d: %s %s (%d), Équipe: %s, Nickname: %s, Points: %d\n",
               i + 1,
               pilotes[i].first_name,
               pilotes[i].last_name,
               pilotes[i].number,
               pilotes[i].team_name,
               pilotes[i].nickname,
               pilotes[i].points);
    }
}

// Tris et affichage du classement des pilotes 
int comparerPoints(const void* a, const void* b) {
    const Pilote* piloteA = (const Pilote*)a;
    const Pilote* piloteB = (const Pilote*)b;

    return piloteB->points - piloteA->points;
}

void afficherClassement(Pilote pilotes[], int taille) {
    qsort(pilotes, taille, sizeof(Pilote), comparerPoints);

    printf("\nClassement des pilotes :\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("| Classement |     Pilote      | Numero |     Equipe      |   Code   | Points |\n");
    printf("--------------------------------------------------------------------------------\n");

    // Afficher chaque pilote
    for (int i = 0; i < taille; i++) {
        printf("| %10d | %-15s | %6d | %-15s | %-7s | %6d |\n",
               i + 1,
               pilotes[i].first_name,  // Combiner Prénom et Nom
               pilotes[i].number,
               pilotes[i].team_name,
               pilotes[i].nickname,
               pilotes[i].points);
    }
    printf("--------------------------------------------------------------------------------\n");
}
// Libérer la mémoire allouée dynamiquement
void libererMemoirePilote(Pilote pilotes[], int taille) {
    for (int i = 0; i < taille; i++) {
        free(pilotes[i].nickname);
        free(pilotes[i].first_name);
        free(pilotes[i].last_name);
        free(pilotes[i].team_name);
    }
}
