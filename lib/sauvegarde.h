#ifndef SAUVEGARDE_H
#define SAUVEGARDE_H

#include "pilotes.h"
#include "voitures.h"

// Prototypes des fonctions
void sauvegarderPilotesCSV(Pilote pilotes[], int taille, const char* filename);
// void afficherSauvegardes(const char* chemin);
char* recupererCheminSauvegarde(const char* chemin);
void enregistrer_practice(const char *nom_course, Voiture voiture[], int nb_voitures, int compteur, SharedData* data);
void enregistrer_qualif_classic(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data);
void enregistrer_qualif_sprint(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data);

#endif // SAUVEGARDE_H
