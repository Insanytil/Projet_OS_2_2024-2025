#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "lib/weekend.h"
#include "lib/voitures.h"
#include "lib/sauvegarde.h"
#include <stdlib.h>
#include <time.h>




#define MIN_TOURS 0
#define MAX_TOURS 30

#define Q1_CLASSIC 4
#define Q2_CLASSIC 4
#define Q3_ClASSIC 2

#define Q1_SPRINT 3
#define Q2_SPRINT 3
#define Q3_SPRINT 2


// Permet d'attendre la validation de l'utilisateur avant de lancer la suite du programme 
    // Attention parfois elle demande de n'appuyer qu'une fois parfois deux fois je n'arrive pas à débugger pour le moment
void attendre_enter() {
    int c;
    printf("\nPress Enter to continue...\n");
    while ((c = getchar()) != '\n' && c != EOF);
    printf("\nPress Enter a second time to confirm...\n");
    getchar();
                          
}
// Permet d'afficher la tableau des meilleurs secteurs et du meilleurs tour
void afficher_meilleurs_temps(SharedData* data) {
    printf("\n         |  Best S1  |  Best S2  |  Best S3  | Best Lap |\n");
    printf("---------------------------------------------------------\n");

    // Ligne des voitures ayant les meilleurs temps
    printf("Car     |    %3d    |    %3d    |    %3d    |   %3d    |\n",
           data->voiture_meilleur_s1,
           data->voiture_meilleur_s2,
           data->voiture_meilleur_s3,
           data->voiture_meilleur_temps);

    printf("---------------------------------------------------------\n");

    // Ligne des meilleurs temps formatés
    printf("Time    | %02d:%02d.%03d | %02d:%02d.%03d | %02d:%02d.%03d | %02d:%02d.%03d |\n",
           data->best_all_s1 / 60000,
           (data->best_all_s1 / 1000) % 60,
           data->best_all_s1 % 1000,
           data->best_all_s2 / 60000,
           (data->best_all_s2 / 1000) % 60,
           data->best_all_s2 % 1000,
           data->best_all_s3 / 60000,
           (data->best_all_s3 / 1000) % 60,
           data->best_all_s3 % 1000,
           data->best_all_time / 60000,
           (data->best_all_time / 1000) % 60,
           data->best_all_time % 1000);

    printf("---------------------------------------------------------\n");
}


// Réinitialise les valeurs des voitures dans la mémoire partagée
void reinitialiser_voitures(Voiture voitures[], int nb_voitures, SharedData* data) {

    // Pour chaque voiture dans la mémoire partagée
    for (int i = 0; i < nb_voitures; i++) {

        voitures[i].bt = 0;             // Meilleur temps remis à zéro
        voitures[i].at = 0;             // Temps total remis à zéro
        voitures[i].nb_tours = 0;       // Nombre de tours remis à zéro
        voitures[i].has_crashed = 0;    // Indicateur de crash réinitialisé
        voitures[i].in_pit = 0;         // Indicateur aux stands réinitialisé
    }

    data->best_all_s1 = 0;              // Meilleurs temps S1 remis à zéro
    data->best_all_s2 = 0;              // Meilleurs temps S2 remis à zéro
    data->best_all_s3 = 0;              // Meilleurs temps S3 remis à zéro
    data->best_all_time = 0;            // Meilleurs temps tour remis à zéro
    data->voiture_meilleur_s1 = -1;     // Numéro de voiture réinitialisé
    data->voiture_meilleur_s2 = -1;     // Numéro de voiture réinitialisé
    data->voiture_meilleur_s3 = -1;     // Numéro de voiture réinitialisé
}

// Permet l'affichage du classement lors d'une séance practice
void afficher_classement_voiture_practice(Voiture voitures[], int taille, SharedData* data) {
    // Effacer l'écran
    system("clear");

    // Copie les voitures dans un tableau temporaire pour le tri
    Voiture classement[taille];
    for (int i = 0; i < taille; i++) {
        classement[i] = voitures[i];
    }

    // Trie les voitures en fonction du meilleur temps (bt)
    for (int i = 0; i < taille - 1; i++) {
        for (int j = i + 1; j < taille; j++) {
            if (classement[i].bt == 0 || (classement[j].bt != 0 && classement[j].bt < classement[i].bt)) {
                Voiture temp = classement[i];
                classement[i] = classement[j];
                classement[j] = temp;
            }
        }
    }

    // Afficher le tableau
    printf("| %-4s | %-6s | %-11s | %-20s | %-7s | %-9s | %-6s | %-21s | %-6s |\n",    // Représente les tailes de colonnes dans le tableau d'affichage
            "Rang", 
            "Numéro", 
            "Code Pilote", 
            "Meilleur Temps", 
            "Crashé", 
            "Au Stand", 
            "Tour(s)", 
            "Temps sur circuit",
            "Etat");
    printf("-------------------------------------------------------------------------------------------------------------\n");

    char bt_formate[16];                               // Buffer pour le meilleur temps formaté
    char at_formate[16];                               // Buffer pour le temps total formaté
    for (int i = 0; i < taille; i++) {
        // Formater le meilleur temps
        formater_temps(classement[i].bt, bt_formate, sizeof(bt_formate));
        formater_temps(classement[i].at, at_formate, sizeof(at_formate));
        const char* statut = (classement[i].in_pit || classement[i].has_crashed) ? "OUT" : "RUN";
        
        printf("| %4d | %6d | %-11s | %-20s | %-7s | %-9s | %6d | %-21s | %-6s |\n",
           i + 1,                                      // Rang
           classement[i].number,                       // Numéro
           classement[i].code_pilote,                  // Code Pilote
           bt_formate,                                 // Meilleur Temps
           classement[i].has_crashed ? "Oui" : "Non",  // Crashé
           classement[i].in_pit ? "Oui" : "Non",       // Au Stand
           classement[i].nb_tours,                     // Tour
           at_formate,                                 // Temps total formaté
           statut);                               
    }
printf("-------------------------------------------------------------------------------------------------------------\n");

afficher_meilleurs_temps(data); // affiche le tableau des meilleurs temps en dessous du tableau de la session

}


// Permet l'affichage du classement lors d'une séance de qualification
void afficher_classement_voiture_qualif(Voiture voitures[], int taille, SharedData* data) {

    // Effacer l'écran
    system("clear");

    // Copier les voitures dans un tableau temporaire pour le tri
    Voiture classement[taille];
    for (int i = 0; i < taille; i++) {
        classement[i] = voitures[i];
    }

    // Trier les voitures en fonction du meilleur temps (bt)
    for (int i = 0; i < taille - 1; i++) {
        for (int j = i + 1; j < taille; j++) {
            if (classement[i].bt == 0 || (classement[j].bt != 0 && classement[j].bt < classement[i].bt)) {
                Voiture temp = classement[i];
                classement[i] = classement[j];
                classement[j] = temp;
            }
        }
    }

    // Afficher le header du tableau
    printf("| %-4s | %-6s | %-11s | %-20s | %-7s | %-9s | %-6s | %-21s | %-6s | %-10s |\n",
           "Rang", 
           "Numéro", 
           "Code Pilote", 
           "Meilleur Temps", 
           "Crashé", 
           "Aux Stands", 
           "Tour(s)", 
           "Temps sur circuit",
           "Etat",
           "Différence");
    printf("-------------------------------------------------------------------------------------------------------------\n");

    char bt_formate[16]; // Buffer pour le meilleur temps formaté
    char at_formate[16]; // Buffer pour le temps total formaté
    char diff_formate[16]; // Buffer pour la différence formatée
    int prev_bt = 0; // Meilleur temps précédent, utilisé pour calculer la différence

    for (int i = 0; i < taille; i++) {  // Boucle pour l'affichage des valeurs des colonnes
        // Formater le meilleur temps
        formater_temps(classement[i].bt, bt_formate, sizeof(bt_formate));
        formater_temps(classement[i].at, at_formate, sizeof(at_formate));

        // Calculer la différence avec le meilleur temps précédent
        if (i > 0 && classement[i].bt > 0 && prev_bt > 0) {
            int diff = classement[i].bt - prev_bt;
            formater_temps(diff, diff_formate, sizeof(diff_formate));
        } else {
            strcpy(diff_formate, "LEADER"); // Pas de différence pour le premier rang alors on lui donne la valeur LEADER
        }

        const char* statut = (classement[i].in_pit || classement[i].has_crashed) ? "ELIM" : "RUN"; // Si la voiture est au mise au PIT ou s'est crashée, elle est donc éliminée

        // Afficher les données pour chaque voiture
        printf("| %4d | %6d | %-11s | %-20s | %-7s | %-9s | %6d | %-21s | %-6s | %-10s |\n",
               i + 1,                                      // Rang
               classement[i].number,                      // Numéro
               classement[i].code_pilote,                 // Code Pilote
               bt_formate,                                // Meilleur Temps
               classement[i].has_crashed ? "Oui" : "Non", // Crashé
               classement[i].in_pit ? "Oui" : "Non",      // Aux Stands
               classement[i].nb_tours,                    // Tour(s)
               at_formate,                                // Temps total formaté
               statut,                                    // Statut
               diff_formate);                             // Différence
               
        // Mettre à jour le meilleur temps précédent
        prev_bt = classement[i].bt;
    }
    printf("-------------------------------------------------------------------------------------------------------------\n");

    afficher_meilleurs_temps(data); // Affiche le tableau des meilleurs temps en dessous du tableau des scores de la session
}


// Permet l'affichage du classement lors d'une course 
void afficher_classement_voiture_course(Voiture voiture[], int taille, SharedData* data) {}


// Gère une séance de practice
void gerer_seance_practice(SharedData* data, int nb_voitures, char* course_name, int compteur) {
    pid_t pids[MAX_VOITURES];
    int tours_restants[MAX_VOITURES]; // Suivi des tours restants par voiture
    key_t sem_key = ftok("practice", 66); // Clé pour le sémaphore
    int semid = initialiser_semaphores(sem_key, 1); // 1 sémaphore pour synchroniser

    printf("Début de la séance de Practice\n");

    // Générer un nombre aléatoire de tours pour chaque voiture
    for (int i = 0; i < nb_voitures; i++) {
        tours_restants[i] = rand() % (MAX_TOURS - MIN_TOURS + 1) + MIN_TOURS;
    }

    // Création des processus enfants
    for (int i = 0; i < nb_voitures; i++) {             // La boucle permet de créer un nombre de processus enfants = aux nombres de voitures pour la séance
        pid_t pid = fork();
        
        if (pid == 0) {                                 // si le processus enfant a bien été créer
                                                        // Processus enfant : Gère une voiture
            srand(time(NULL) + getpid());               // Permet une seed personelle 
            while (tours_restants[i] > 0) {
                verrouiller_semaphore(semid, 0);        // verouille le sémaphore
                gerer_tour(&data->voitures[i], data, i);//Permet la gestion d'un tour 
                tours_restants[i]--; // décrémente le nombre de tour restant pour la session practice
                deverrouiller_semaphore(semid, 0);
                usleep(200000);
            }
            data->voitures[i].in_pit = 1;
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            pids[i] = pid;                              // Sauvegarde le PID de l’enfant
        } else {
            perror("Erreur lors de la création du processus");
            exit(EXIT_FAILURE);
        }
    }

    // Processus parent : Coordonne les tours

        
    int voitures_actives = nb_voitures;                 // Initialisatio d'un compteur permettant de vérifier combien de voitures sont encores actives


    // Boucle qui tourne tant qu'il y a au moins une voiture active
    while (voitures_actives > 0) {
        // Boucle parcourant un tableau contenant les PID des processus enfants, si Pid > 0 alors l'enfant est encore en train de tourner
        for (int i = 0; i < nb_voitures; i++) {
            if (pids[i] > 0) {
                int status;
                pid_t result = waitpid(pids[i], &status, WNOHANG); // WNOHANG indique que waitpid ne doit pas bloquer et return si le processus enfant n'est pas terminer
                if (result > 0) {
                    if (WIFEXITED(status)) {                    // Si waitpid retourne un résultat positif (cela signifie que l'enfant a terminé), on vérifie avec WIFEXITED(status) si l'enfant s'est terminé normalement
                        pids[i] = -1;           // Si le processus s'est terminé normalement on marque le PID correspondant à -1 
                        voitures_actives--;        // on décrémente le nombres de voitures encore actives pour indiquer qu'un processus à fini son execution
                    }
                }
            }
        }

        afficher_classement_voiture_practice(data->voitures, nb_voitures, data); // affiche le classement de la séance de practice
        usleep(500000); // afin d'éviter un affichage trop intense
    }

    // Suppression des sémaphores après la séance
    detruire_semaphores(semid);

    printf("Séance de Practice terminée.\n");

    // On affiche le classement final et de manière fixe
    afficher_classement_voiture_practice(data->voitures, nb_voitures, data); 

    // On enregistre les données de la séance dans le fichier adéquat
    enregistrer_practice(course_name, data->voitures, nb_voitures, compteur, data);

    // On réinitialise les informations de la mémoire partagée pour les prochaines séances
    reinitialiser_voitures(data->voitures, nb_voitures, data);
    
}

// Gérer une séance de qualification (Sprint et Classique)
void gerer_qualifications_classiques(SharedData* data, int nb_voitures, int nb_elimines, int duree_session) {
    printf("\nDébut des qualifications classiques\n");

    // Initialisation des variables nécessaires
    pid_t pids[MAX_VOITURES];
    key_t sem_key = ftok("qualifications", 66);
    int semid = initialiser_semaphores(sem_key, 1);
    time_t debut_session = time(NULL); // Début de la session

    // Création des processus enfants
    for (int i = 0; i < nb_voitures; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Processus enfant : Gère une voiture
            srand(time(NULL) + getpid()); // Graine unique pour chaque processus

            while (1) {
                verrouiller_semaphore(semid, 0);

                // Vérifiez si le temps de la session est terminé
                if (time(NULL) - debut_session > duree_session) {
                    deverrouiller_semaphore(semid, 0);
                    break; // Sortir de la boucle si le temps est écoulé
                }

                gerer_tour(&data->voitures[i], data, i); // Simule un tour
                deverrouiller_semaphore(semid, 0);

                usleep(500000); // Pause pour éviter une boucle trop rapide
            }

            exit(EXIT_SUCCESS); // Quitte le processus après la fin des tours
        } else if (pid > 0) {
            pids[i] = pid; // Sauvegarde le PID de l’enfant
        } else {
            perror("Erreur lors de la création du processus");
            exit(EXIT_FAILURE);
        }
    }

    // Processus parent : Coordination
    int voitures_actives = nb_voitures;
    while (voitures_actives > 0) {
        for (int i = 0; i < nb_voitures; i++) {
            if (pids[i] > 0) {
                int status;
                pid_t result = waitpid(pids[i], &status, WNOHANG);
                if (result > 0) {
                    if (WIFEXITED(status)) {
                        pids[i] = -1; // Marquer le processus comme terminé
                        voitures_actives--;
                    }
                }
            }
        }

        afficher_classement_voiture_qualif(data->voitures, nb_voitures, data); // Affiche le classement
        usleep(400000); // Pause pour éviter trop de rafraîchissements
    }

    // Trier et marquer les voitures les plus lentes comme éliminées
    trier_voitures_qualifs(data->voitures, nb_voitures);
    for (int i = nb_voitures - nb_elimines; i < nb_voitures; i++) {
        data->voitures[i].in_pit = 1;
        printf("Voiture %d éliminée.\n", data->voitures[i].number);
    }

    printf("\nSession terminée. Classement mis à jour.\n");
    afficher_classement_voiture_qualif(data->voitures, nb_voitures, data);
    detruire_semaphores(semid);
}

// Permet de Gérer la simulation d'un Weekend qu'il soit Spécial ou Classique
void gerer_weekend(WeekendType type, SharedData* data, int nb_voitures, int nb_tours_course, char* course_name) {
    system("clear");
    printf("Début du weekend de course (%s)\n", type == CLASSIC ? "CLASSIQUE" : "SPECIAL");
    int compteur_practice = 0;

    // Jour 1 : Vendredi
    
    char choixUn;
    char choixDeux;
    char choixTrois;
    printf("Voulez vous lancer la Séance P1 ? [o/n]\n");
    


    scanf(" %c", &choixUn);
    if (choixUn == 'o') {
        compteur_practice = 1;
        gerer_seance_practice(data, nb_voitures, course_name, compteur_practice);  
    } else {
        printf("\nLa séance P1 n'a pas été executée\n");
    }


    printf("Voulez-vous lancer la Séance P2 ? [o/n]\n");
    
    scanf(" %c", &choixDeux);
    if (choixDeux == 'o') {
        compteur_practice = 2;
    gerer_seance_practice(data, nb_voitures, course_name, compteur_practice);
    } else {
        printf("\nLa séance P2 n'a pas été executéee\n");
    }



    // Jour 2 : Samedi
    
    if (type == CLASSIC) {
        printf("Voulez-vous lancer la Séance P3 ? [o/n]\n");
        
        scanf(" %c", &choixTrois);
        if (choixTrois == 'o') {
            compteur_practice = 3;
        gerer_seance_practice(data, nb_voitures, course_name, compteur_practice);
        } else {
            printf("La séance P3 n'a pas été executée\n");
        }

        printf("Qualifications classiques (Q1, Q2, Q3)\n");
        printf("Lancement de Q1 :\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 5, Q1_CLASSIC);

        printf("Lancement de Q2 :\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 10, Q2_CLASSIC);

        printf("Lancement de Q3 :\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 0, Q3_ClASSIC);

        printf("Fin des qualifications \n");
        
        attendre_enter();
        

        enregistrer_qualif_classic(course_name, data->voitures, nb_voitures, data);
        reinitialiser_voitures(data->voitures, nb_voitures, data);
        
    } else {

        printf("Lancer Sprint Shootout (Qualifications 1 pour le sprint)\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 5, Q1_SPRINT);

        printf("Lancer Sprint Shootout (Qualifications 2 pour le sprint)\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 10, Q2_SPRINT);

        printf("Lancer Sprint Shootout (Qualifications 3 pour le sprint)\n");
        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 0, Q3_SPRINT);

        enregistrer_qualif_sprint(course_name, data->voitures, nb_voitures, data);

        reinitialiser_voitures(data->voitures, nb_voitures, data);


        printf("Fin des qualifications, lancement du sprint \n");
        
        attendre_enter();
        

        printf("Course Sprint (~100 km)\n");
        //gerer_sprint(data, nb_voitures);

        printf("Qualifications pour la course principale (Q1, Q2, Q3)\n");
        
        
        gerer_qualifications_classiques(data, nb_voitures, 5, Q1_CLASSIC);

        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 10, Q2_CLASSIC);

        
        attendre_enter();
        

        gerer_qualifications_classiques(data, nb_voitures, 0, Q3_ClASSIC);

        printf("Qualifications pour la course terminées\n");
        attendre_enter();


        enregistrer_qualif_classic(course_name, data->voitures, nb_voitures, data);
        reinitialiser_voitures(data->voitures, nb_voitures, data);
        system("clear");
        
    }

    // Jour 3 : Dimanche
    printf("\n--- Jour 3 : Dimanche ---\n");
    printf("Course principale\n");
    // gerer_course(data, nb_voitures, nb_tours_course);

    printf("Weekend terminé.\n");
}
