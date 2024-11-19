#include "lib/voitures.h"
#include <stdio.h>
#include <string.h>

// Génère un temps aléatoire pour un secteur
int get_car_time() {
    int MIN_TEMPS = 25000;
    int MAX_TEMPS = 45000;
    return rand() % (MAX_TEMPS - MIN_TEMPS + 1) + MIN_TEMPS;
}

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

// Gestion de la mémoire partagée
SharedData* initialiser_mem_partagee(int* shmid) {
    key_t key = ftok("practice", 65); // Générer une clé unique
    *shmid = shmget(key, sizeof(SharedData), 0666 | IPC_CREAT);

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