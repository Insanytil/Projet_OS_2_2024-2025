#ifndef PILOTE_H
#define PILOTE_H

// Définition de la structure Pilote
typedef struct {
    int number;
    char* nickname;
    char* first_name;
    char* last_name;
    char* team_name;
    int points;
} Pilote;

// Prototypes des fonctions
void initPilotes(Pilote pilotes[], const char* filename);
void afficherPilotes(Pilote pilotes[], int taille);
void libererMemoirePilote(Pilote pilotes[], int taille);

int comparerPoints(const void* a, const void* b);

void afficherClassement(Pilote pilotes[], int taille);

#endif // PILOTE_H
