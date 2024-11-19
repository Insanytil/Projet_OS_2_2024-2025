#ifndef WEEKEND_H
#define WEEKEND_H

#include "voitures.h"
#include "sauvegarde.h"

typedef enum {
    CLASSIC,
    SPECIAL
} WeekendType;

void gerer_weekend(WeekendType type, SharedData* data, int nb_voitures, int nb_tours_cours, char* nom_course);
void gerer_seance_practice(SharedData* data, int nb_voitures, char* course_name, int compteur);
void afficher_classement_voiture_practice(Voiture voitures[], int taille, SharedData* data);
void afficher_classement_voiture_qualif(Voiture voitures[], int taille, SharedData* data);
void reinitialiser_voitures(Voiture voitures[], int nb_voitures, SharedData* data);
void afficher_meilleurs_temps(SharedData* data);

//void gerer_qualifications_classiques(SharedData* data, int nb_voitures);
//void gerer_qualifications_speciales(SharedData* data, int nb_voitures);
//void gerer_sprint(SharedData* data, int nb_voitures);
//void gerer_course(SharedData* data, int nb_voitures, int nb_tours_course);

#endif