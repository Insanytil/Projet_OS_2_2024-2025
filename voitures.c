#include "lib/voitures.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


// Génère un temps aléatoire pour un secteur
int get_car_time() {
    int MIN_TEMPS = 25000;
    int MAX_TEMPS = 30000;
    return random() % (MAX_TEMPS - MIN_TEMPS + 1) + MIN_TEMPS;
}

// Génère un temps pour le pit stop
int get_pit_time() {
    return 20000 + (random() % 10000); // Temps entre 20000ms et 40000ms (20-40 secondes)
}


// Permet de formatter l'affichage du temps
void formater_temps(int temps_ms, char* buffer, size_t buffer_size) {
    if (temps_ms <= 0) {
        snprintf(buffer, buffer_size, "N/A");
        return;
    }

    int minutes = temps_ms / 60000;
    int secondes = (temps_ms % 60000) / 1000;
    int millisecondes = temps_ms % 1000;

    snprintf(buffer, buffer_size, "%02d:%02d.%03d", minutes, secondes, millisecondes);
}

// Permet de formatter l'affichage du temps passé pendant un pit stop
void formater_temps_pit(int temps_ms, char* buffer, size_t taille_buffer) {
    int secondes = temps_ms / 1000;                 // Calculer les secondes
    int millisecondes = temps_ms % 1000;           // Calculer les millisecondes restantes

    // Formater le temps en +(ss.mmm)
    snprintf(buffer, taille_buffer, "+(%02d.%03d)", secondes, millisecondes);
}

// défini la probabilité pour qu'une voiture fasse un pit stop
int doit_aller_au_stand(Voiture* voiture, int nb_tours_total) {
    double proba_pit = 0.0;
    proba_pit = voiture->tours_depuis_pit * 0.5; // +0.5% par tour sans pit

    if (voiture->tours_depuis_pit >= 40 && voiture->has_pit < 1) {
        proba_pit = 30.0; // 30% si jamais passé au pit et après 40 tours
    }

    if (proba_pit > 30.0) {
        proba_pit = 30.0;
    }

    srand(time(NULL) ^ getpid());
    double random_value = (double)rand() / RAND_MAX * 100;

    return random_value <= proba_pit;
}

// Initialise une voiture avec un numéro et un code pilote
void init_voiture(Voiture* voiture, int number, const char* code_pilote) {
    voiture->number = number;
    voiture->s1 = 0;
    voiture->s2 = 0;
    voiture->s3 = 0;
    voiture->bt = 0;
    voiture->tt = 0;
    voiture->has_crashed = 0;
    voiture->in_pit = 0;
    strncpy(voiture->code_pilote, code_pilote, 3); // Copie le code pilote (3 caractères max)
    voiture->code_pilote[3] = '\0'; // Assure la terminaison
    voiture->nb_tours = 0;
    voiture->at = 0;
    voiture->has_pit = 0;
    voiture->tours_depuis_pit = 0;
    voiture->time_in_pit = 0;
}

// Simule un tour pour une voiture
void gerer_tour(Voiture* voiture, SharedData* data, int id) {
    // Si la voiture est aux stands
    if (voiture->in_pit) {
        return;
    }

    // Générer les temps pour chaque secteur
    voiture->s1 = get_car_time();
    voiture->s2 = get_car_time();
    voiture->s3 = get_car_time();

    // Calculer le temps total pour le tour
    voiture->tt = voiture->s1 + voiture->s2 + voiture->s3;
    voiture->at += voiture->tt;
    voiture->nb_tours += 1;
    // Mettre à jour le meilleur temps global
    if (data->best_all_time == 0 || voiture->tt < data->best_all_time) {
        data->best_all_time = voiture->tt;
        data->voiture_meilleur_temps = voiture->number;
        
    }

    // Mettre à jour le meilleur temps du secteur 1
    if (data->best_all_s1 == 0 || voiture->s1 < data->best_all_s1) {
        data->best_all_s1 = voiture->s1;
        data->voiture_meilleur_s1 = voiture->number;
        
    }

    // Mettre à jour le meilleur temps du secteur 2
    if (data->best_all_s2 == 0 || voiture->s2 < data->best_all_s2) {
        data->best_all_s2 = voiture->s2;
        data->voiture_meilleur_s2 = voiture->number;
        
    }

    // Mettre à jour le meilleur temps du secteur 3
    if (data->best_all_s3 == 0 || voiture->s3 < data->best_all_s3) {
        data->best_all_s3 = voiture->s3;
        data->voiture_meilleur_s3 = voiture->number;
        
    }

    // Mettre à jour le meilleur temps de la voiture
    if (voiture->bt == 0 || voiture->tt < voiture->bt) {
        voiture->bt = voiture->tt;
        data->nouveau_meilleur_temps[id] = 1; // Signale un nouveau meilleur temps
    }
    

}

// Simule un tour pour une course/sprint
void gerer_tour_course(Voiture* voiture, SharedData* data, int id, int nb_tours) {
    if (!voiture->has_crashed) {
        srand(time(NULL) * getpid() + voiture->number);
        if ((random() % 1000) < 5) { // 0.5% chance de crash par tour
            voiture->has_crashed = 1;
        }
    }

    if (voiture->has_crashed) {
        voiture->at = 9999999;
        voiture->nb_tours++;
        return;
    }

    // Obtenir les temps bruts pour chaque secteur
    voiture->s1 = get_car_time();
    voiture->s2 = get_car_time();
    voiture->s3 = get_car_time();

     // Calculer le seuil pour désactiver le facteur
    int seuil = nb_tours / 5;

    // Calculer le facteur de rapidité
    float facteur_rapidite;

    // Calculer le facteur de rapidité en fonction du classement
    if (voiture->nb_tours <= seuil) {
    float facteur_base = ((float)(id - 1) / 500.0);
    float degressif = 1.0 - ((float)voiture->nb_tours / (float)seuil);
    facteur_rapidite = 1.0 + (facteur_base * degressif);
    } else {
    facteur_rapidite = 1.0; // Pas de malus après le seuil
    }

    // Appliquer le facteur de rapidité sur chaque secteur
    voiture->s1 = (int)(voiture->s1 * facteur_rapidite);
    voiture->s2 = (int)(voiture->s2 * facteur_rapidite);
    voiture->s3 = (int)(voiture->s3 * facteur_rapidite);

    // Gestion des arrêts au stand
    int pit_time = 0;
    if (doit_aller_au_stand(voiture, nb_tours)) {
        pit_time = get_pit_time();
        voiture->in_pit = 1;
        voiture->tours_depuis_pit = 0;
        voiture->has_pit++;
    } else {
        voiture->tours_depuis_pit++;
    }

    // Ajouter le temps de pit stop au secteur 3
    voiture->s3 += pit_time;
    voiture->time_in_pit = pit_time;

    // Calculer le temps total du tour
    voiture->tt = voiture->s1 + voiture->s2 + voiture->s3;
    voiture->at += voiture->tt;
    voiture->nb_tours++;

    // Mettre à jour le meilleur temps global
    if (data->best_all_time == 0 || voiture->tt < data->best_all_time) {
        data->best_all_time = voiture->tt;
        data->voiture_meilleur_temps = voiture->number;
    }

    // Mettre à jour le meilleur temps du secteur 1
    if (data->best_all_s1 == 0 || voiture->s1 < data->best_all_s1) {
        data->best_all_s1 = voiture->s1;
        data->voiture_meilleur_s1 = voiture->number;
    }

    // Mettre à jour le meilleur temps du secteur 2
    if (data->best_all_s2 == 0 || voiture->s2 < data->best_all_s2) {
        data->best_all_s2 = voiture->s2;
        data->voiture_meilleur_s2 = voiture->number;
    }

    // Mettre à jour le meilleur temps du secteur 3
    if (data->best_all_s3 == 0 || voiture->s3 < data->best_all_s3) {
        data->best_all_s3 = voiture->s3;
        data->voiture_meilleur_s3 = voiture->number;
    }

    // Mettre à jour le meilleur temps de la voiture
    if (voiture->bt == 0 || voiture->tt < voiture->bt) {
        voiture->bt = voiture->tt;
        data->nouveau_meilleur_temps[id] = 1; // Signale un nouveau meilleur temps
    }
}

// Gestion de la mémoire partagée
SharedData* initialiser_mem_partagee(int* shmid) {
    key_t key = ftok("courses.c", 65); // Générer une clé unique
    if (key == -1) {
        perror("Erreur lors de la génération de la clé avec ftok");
        exit(EXIT_FAILURE);
    }
    *shmid = shmget(key, sizeof(SharedData), 0666 | IPC_CREAT);
    printf("On est dans la mémoire partagée\n");
    if (*shmid == -1) {

        perror("Erreur lors de la création de la mémoire partagée");
        exit(EXIT_FAILURE);
    }

    SharedData* data = (SharedData*)shmat(*shmid, NULL, 0);

    if (data == (void*)-1) {
        perror("Erreur lors de l'attachement de la mémoire partagée");
        exit(EXIT_FAILURE);
    }

    // Initialiser les données partagées
    memset(data->processus_termines, 0, sizeof(data->processus_termines));
    memset(data->nouveau_meilleur_temps, 0, sizeof(data->nouveau_meilleur_temps));


    data->best_all_time = 0; // Pas encore de meilleur temps global
    data->voiture_meilleur_temps = -1; // Aucune voiture associée au meilleur temps global

    data->best_all_s1 = 0; // Pas encore de meilleur temps pour S1
    data->voiture_meilleur_s1 = -1; // Aucune voiture associée au meilleur temps de S1

    data->best_all_s2 = 0; // Pas encore de meilleur temps pour S2
    data->voiture_meilleur_s2 = -1; // Aucune voiture associée au meilleur temps de S2

    data->best_all_s3 = 0; // Pas encore de meilleur temps pour S3
    data->voiture_meilleur_s3 = -1; // Aucune voiture associée au meilleur temps de S3


    return data;
}

void detacher_mem_partagee(SharedData* data, int shmid) {
    if (shmdt(data) == -1) {
        perror("Erreur lors du détachement de la mémoire partagée");
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la suppression de la mémoire partagée");
    }
}

// Initialiser les sémaphores
int initialiser_semaphores(key_t key, int nb_semaphores) {
    int semid = semget(key, nb_semaphores, 0666 | IPC_CREAT);

    if (semid == -1) {
        perror("Erreur lors de la création des sémaphores");
        exit(EXIT_FAILURE);
    }

    // Initialiser les valeurs des sémaphores à 1
    for (int i = 0; i < nb_semaphores; i++) {
        if (semctl(semid, i, SETVAL, 1) == -1) {
            perror("Erreur lors de l'initialisation des sémaphores");
            exit(EXIT_FAILURE);
        }
    }

    return semid;
}

// Verrouiller un sémaphore
void verrouiller_semaphore(int semid, int semaphore_index) {
    struct sembuf operation = {semaphore_index, -1, 0};
    if (semop(semid, &operation, 1) == -1) {
        perror("Erreur lors du verrouillage du sémaphore");
        exit(EXIT_FAILURE);
    }
}

// Déverrouiller un sémaphore
void deverrouiller_semaphore(int semid, int semaphore_index) {
    struct sembuf operation = {semaphore_index, 1, 0};
    if (semop(semid, &operation, 1) == -1) {
        perror("Erreur lors du déverrouillage du sémaphore");
        exit(EXIT_FAILURE);
    }
}

// Détruire les sémaphores
void detruire_semaphores(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Erreur lors de la suppression des sémaphores");
    }
}

void trier_voitures_qualifs(Voiture voitures[], int nb_voitures) {
    for (int i = 0; i < nb_voitures - 1; i++) {
        for (int j = 0; j < nb_voitures - i - 1; j++) {
            // Vérifier les temps et trier dans l'ordre croissant
            if (voitures[j].bt == 0 || 
               (voitures[j + 1].bt != 0 && voitures[j + 1].bt < voitures[j].bt)) {
                // Échanger les deux structures
                Voiture temp = voitures[j];
                voitures[j] = voitures[j + 1];
                voitures[j + 1] = temp;
            }
        }
    }
}

void trier_voitures_course(Voiture voitures[], int nb_voitures) {
    for (int i = 0; i < nb_voitures - 1; i++) {
        for (int j = 0; j < nb_voitures - i - 1; j++) {
            // Vérifier les temps et trier dans l'ordre croissant
            if (voitures[j].at == 0 || 
               (voitures[j + 1].at != 0 && voitures[j + 1].at < voitures[j].at)) {
                // Échanger les deux structures
                Voiture temp = voitures[j];
                voitures[j] = voitures[j + 1];
                voitures[j + 1] = temp;
            }
        }
    }
}