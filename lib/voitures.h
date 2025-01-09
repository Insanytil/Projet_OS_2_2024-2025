#ifndef VOITURE_H
#define VOITURE_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>

#include "courses.h"

// Constantes
#define MAX_VOITURES 20

// Structure de la voiture
typedef struct {
    int number;         // Numéro de la voiture
    int s1;             // Temps secteur 1
    int s2;             // Temps secteur 2
    int s3;             // Temps secteur 3
    int bt;             // Meilleur temps
    int tt;             // Temps total
    int has_crashed;    // 1 si la voiture a crashé, 0 sinon
    int in_pit;         // 1 si la voiture est aux stands, 0 sinon
    int has_pit;        // 1 si la voiture a effectué au moins un pit dans la course
    char code_pilote[4]; // Code pilote
    int nb_tours;       // nombre de tours effectué par la voiture
    int at;             // temps total depuis que la voiture roule sur circuit
    int tours_depuis_pit; // représente le nombre de tour depuis le dernier pit
    int time_in_pit;      // représente le temps passer dans le pit
} Voiture;

// Structure de données partagée
typedef struct {
    Voiture voitures[MAX_VOITURES];  //Données des voitures
    int processus_termines[MAX_VOITURES];  // 1 si le processus a terminé son tour
    int nouveau_meilleur_temps[MAX_VOITURES]; // 1 si un meilleur temps a été établi
    int best_all_time;
    int voiture_meilleur_temps;

    int best_all_s1;
    int voiture_meilleur_s1;

    int best_all_s2;
    int voiture_meilleur_s2;

    int best_all_s3; 
    int voiture_meilleur_s3;

} SharedData;

// Fonctions principales pour les voitures
int get_car_time(); // Génère un temps aléatoire
int get_pit_time(); // Génère un temps pour le pit stop
void init_voiture(Voiture* voiture, int number, const char* code_pilote); // Initialise une voiture
void gerer_tour(Voiture* voiture, SharedData* data, int id); // Simule un tour pour une voiture
void gerer_tour_course(Voiture* voiture, SharedData* data, int id, int nb_tours); // Simule un tour pour la course ou le sprint
// Gestion de la mémoire partagée
SharedData* initialiser_mem_partagee(int* shmid);
void detacher_mem_partagee(SharedData* data, int shmid);

// Gestion des sémaphores
int initialiser_semaphores(key_t key, int nb_semaphores);
void verrouiller_semaphore(int semid, int semaphore_index);
void deverrouiller_semaphore(int semid, int semaphore_index);
void detruire_semaphores(int semid);
void formater_temps(int temps_ms, char* buffer, size_t buffer_size);
void formater_temps_pit(int temps_ms, char* buffer, size_t taille_buffer) ;
void trier_voitures_qualifs(Voiture voitures[], int nb_voitures);
void trier_voitures_course(Voiture voitures[], int nb_voitures);
#endif
