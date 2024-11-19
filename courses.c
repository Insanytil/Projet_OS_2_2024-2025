#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lib/courses.h"
#include "lib/pilotes.h"
#include "lib/weekend.h"
#include "lib/voitures.h"

// Fonction pour initialiser les courses depuis le fichier CSV
void initCourses(Course courses[], const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int i = 0;

    // Lire chaque ligne du fichier CSV
    while (fgets(line, sizeof(line), file) && i < 25) {
        // Suppression du saut de ligne
        line[strcspn(line, "\n")] = '\0';

        // Découper la ligne en utilisant la virgule comme séparateur
        char* token = strtok(line, ",");
        courses[i].name = strdup(token);

        token = strtok(NULL, ",");
        courses[i].country = strdup(token);

        token = strtok(NULL, ",");
        courses[i].nb_laps = atoi(token);

        token = strtok(NULL, ",");
        courses[i].km_lap = atoi(token);

        token = strtok(NULL, ",");
        courses[i].km_race = atoi(token);

        token = strtok(NULL, ",");
        courses[i].sprint = atoi(token); // Conversion directe de 0 (false) ou 1 (true)

        i++;
    }

    fclose(file);
}

// Fonction pour afficher les informations des courses

void afficherCoursesDisponibles(Course courses[], int taille) {
    system("clear");
    printf("\nCourses disponibles :\n");
    printf("-----------------------------------------------------------------------\n");
    printf("| Numero |                 Nom                  |         Pays        |\n");
    printf("-----------------------------------------------------------------------\n");

    for (int i = 0; i < taille; i++) {
        printf("| %6d | %-36s | %-19s |\n",
               i + 1,
               courses[i].name,
               courses[i].country);
    }

    printf("-----------------------------------------------------------------------\n");
    printf("  0 - Retour au menu principal\n");
}


// FOnction simulant une course 
void simulerCourse(Course courses[], int taille_courses, Pilote pilotes[], int taille_pilotes) {
    int choix_course = -1;

    while (1) {
        // Afficher les courses disponibles
        afficherCoursesDisponibles(courses, taille_courses);

        // Demander le choix de l'utilisateur
        printf("\nSélectionnez une course (0 pour revenir au menu principal) : ");
        scanf("%d", &choix_course);

        // Vérifier le choix
        if (choix_course == 0) {
            printf("\nRetour au menu principal.\n");
            return; // Retour au menu secondaire
        }

        if (choix_course > 0 && choix_course <= taille_courses) {
            Course course_selectionnee = courses[choix_course - 1];

            // Vérifier si la course est un sprint
            if (course_selectionnee.sprint) {
                printf("\nSimulation d'une course Sprint : %s, %s\n",
                       course_selectionnee.name, course_selectionnee.country);
                 lancerSimulationWeekendSpecial(course_selectionnee, pilotes, taille_pilotes);

                // lancerSimulationWeekendSpecial(course_selectionnee, pilotes, taille_pilotes);


            } else {
                printf("\nSimulation d'une course Classique : %s, %s\n",
                       course_selectionnee.name, course_selectionnee.country);
                        lancerSimulationWeekendClassic(course_selectionnee, pilotes, taille_pilotes);


                // lancerSimulationWeekendClassic(course_selectionnee, pilotes, taille_pilotes);


            }

            printf("Simulation terminée.\n");
            return; // Retourner au menu secondaire après simulation
        } else {
            printf("\nChoix invalide. Veuillez sélectionner une course valide.\n");
        }
    }
}


void lancerSimulationWeekendClassic(Course course, Pilote pilotes[], int taille_pilotes) {
    // Initialiser la mémoire partagée et les sémaphores
    int shmid;
    SharedData* data = initialiser_mem_partagee(&shmid);

    // Initialiser les voitures à partir des pilotes
    for (int i = 0; i < taille_pilotes; i++) {
        init_voiture(&data->voitures[i], pilotes[i].number, pilotes[i].nickname);
    }

    // Lancer un weekend classique
    gerer_weekend(CLASSIC, data, taille_pilotes, course.nb_laps, course.name);

    // Libérer les ressources
    detacher_mem_partagee(data, shmid);
}


void lancerSimulationWeekendSpecial(Course course, Pilote pilotes[], int taille_pilotes) {
    // Initialiser la mémoire partagée et les sémaphores
    int shmid;
    SharedData* data = initialiser_mem_partagee(&shmid);

    // Initialiser les voitures à partir des pilotes
    for (int i = 0; i < taille_pilotes; i++) {
        init_voiture(&data->voitures[i], pilotes[i].number, pilotes[i].nickname);
    }

    // Lancer un weekend spécial
    gerer_weekend(SPECIAL, data, taille_pilotes, course.nb_laps, course.name);

    // Libérer les ressources
    detacher_mem_partagee(data, shmid);
}
// Fonction pour libérer la mémoire des courses
void libererMemoireCourses(Course courses[], int taille) {
    for (int i = 0; i < taille; i++) {
        free(courses[i].name);
        free(courses[i].country);
    }
}
