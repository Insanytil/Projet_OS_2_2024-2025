#ifndef WEEKEND_H
#define WEEKEND_H

#include "voitures.h"
#include "sauvegarde.h"
#include "courses.h"
#include "pilotes.h"

typedef enum {
    CLASSIC,
    SPECIAL
} WeekendType;

void gerer_weekend(WeekendType type, SharedData* data, int nb_voitures, Course course, Pilote pilotes[]);
void gerer_seance_practice(SharedData* data, int nb_voitures, char* course_name, int compteur);
void afficher_classement_voiture_practice(Voiture voitures[], int taille, SharedData* data);
void afficher_classement_voiture_qualif(Voiture voitures[], int taille, SharedData* data);
void reinitialiser_voitures(Voiture voitures[], int nb_voitures, SharedData* data);
void afficher_meilleurs_temps(SharedData* data);
void attribuer_points_post_sprint(Pilote pilotes[], SharedData* data);
void attribuer_points_post_course(Pilote pilotes[], SharedData* data);


#endif