#ifndef COURSE_H
#define COURSE_H

#include "pilotes.h"

// Définition de la structure Course
typedef struct {
    char* name;
    char* country;
    int nb_laps;
    int km_lap;
    int km_race;
    int sprint;
} Course;

// Prototypes des fonctions
void initCourses(Course courses[], const char* filename);
void afficherCoursesDisponibles(Course courses[], int taille);
void simulerCourse(Course courses[], int taille_courses, Pilote pilotes[], int taille_pilotes);
void libererMemoireCourses(Course courses[], int taille);
void lancerSimulationWeekendClassic(Course course, Pilote pilotes[], int taille_pilotes);
void lancerSimulationWeekendSpecial(Course course, Pilote pilotes[], int taille_pilotes);

#endif // COURSE_H
