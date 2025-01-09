#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "lib/weekend.h"
#include "lib/voitures.h"
#include "lib/pilotes.h"
#include "lib/sauvegarde.h"
#include <stdlib.h>
#include <time.h>



#define MIN_TOURS 0
#define MAX_TOURS 30

#define Q1_CLASSIC 4
#define Q2_CLASSIC 4
#define Q3_CLASSIC 2

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
        voitures[i].has_pit = 0;        // Indicateur de nombre de passage au pit réinitialisé
        voitures[i].tours_depuis_pit = 0; // Indicateur de nombre de tours depuis le dernier pit réinitilisé
        voitures[i].time_in_pit = 0;        //  temps passer au pit remis à zéro
        voitures[i].tt = 0;             // temps total mis pour le tour remis à zéro
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
    printf("| %-4s | %-6s | %-11s | %-20s | %-9s | %-6s | %-21s | %-6s | %-10s |\n",
           "Rang", 
           "Numéro", 
           "Code Pilote", 
           "Meilleur Temps",  
           "Au Stand", 
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

        const char* statut = (classement[i].in_pit || classement[i].has_crashed) ? "ELIM" : classement[i].time_in_pit ? "END" : "RUN"; // Si la voiture est au mise au PIT ou s'est crashée, elle est donc éliminée

        // Afficher les données pour chaque voiture
        if (classement[i].in_pit) {
            // Affiche toute la ligne en rouge
            printf("\033[31m");  // Début de la couleur rouge
        }

        printf("| %4d | %6d | %-11s | %-20s | %-9s | %6d | %-21s | %-6s | %-10s |\n",
            i + 1,                                                                         // Rang
            classement[i].number,                                                         // Numéro
            classement[i].code_pilote,                                                   // Code Pilote
            classement[i].in_pit ? "ELIM" : bt_formate,                                 // Meilleur Temps
            classement[i].in_pit ? "Oui" : "Non",                                      // Au Stand
            classement[i].nb_tours,                                                   // Tour(s)
            classement[i].in_pit ? "ELIM" : at_formate,                              // Temps total formaté
            statut,                                                                 // Statut
            classement[i].in_pit ? "ELIM" : diff_formate                             // Différence formatée
        );

        if (classement[i].in_pit) {
            // Réinitialise la couleur après avoir imprimé la ligne
            printf("\033[0m");
        }
        // Mettre à jour le meilleur temps précédent
        prev_bt = classement[i].bt;
    }
    printf("-------------------------------------------------------------------------------------------------------------\n");

    afficher_meilleurs_temps(data); // Affiche le tableau des meilleurs temps en dessous du tableau des scores de la session
}


// Permet l'affichage du classement lors d'une course 
void afficher_classement_voiture_sprint(Voiture* voitures, int taille, SharedData* data, int nb_lap) {
    // Effacer l'écran
    system("clear");

    // Trier les voitures en fonction du temps total
    Voiture classement[taille];
    for (int i = 0; i < taille; i++) {
        classement[i] = voitures[i];
    }

    for (int i = 0; i < taille - 1; i++) {
        for (int j = i + 1; j < taille; j++) {
            if (classement[j].at > 0 && (classement[i].at == 0 || classement[j].at < classement[i].at)) {
                Voiture temp = classement[i];
                classement[i] = classement[j];
                classement[j] = temp;
            }
        }
    }

    // Afficher le classement
    printf("| %-4s | %-6s | %-11s | %-13s | %-7s | %-17s | %-7s | %-10s | %-10s |\n",
           "Rang", "Numéro", "Code Pilote", "Meilleur Temps", "Tour(s)", 
           "Temps Circuit", "Etat", "Différence", "Pit");
    printf("------------------------------------------------------------------------------------------------------------------\n");

    char bt_formate[16], at_formate[16], diff_formate[16], tour_formate[16], pit_time[16];
    int prev_at = 0;

    for (int i = 0; i < taille; i++) {
        formater_temps(classement[i].bt, bt_formate, sizeof(bt_formate));
        formater_temps(classement[i].at, at_formate, sizeof(at_formate));
        formater_temps_pit(classement[i].time_in_pit, pit_time, sizeof(pit_time));
        snprintf(tour_formate, sizeof(tour_formate), "%d / %d", classement[i].nb_tours, nb_lap);

        if (i > 0 && classement[i].at > 0 && prev_at > 0) {
            formater_temps(classement[i].at - prev_at, diff_formate, sizeof(diff_formate));
        } else {
            strcpy(diff_formate, "LEADER");
        }

        printf("| %-4d | %-6d | %-11s | %-13s | %-7s | %-17s | %-7s | %-10s | %-10s |\n",
                i + 1,
                classement[i].number, 
                classement[i].code_pilote, 
                classement[i].has_crashed ? "--:--:--" : bt_formate,
                classement[i].has_crashed ? "---" : tour_formate, 
                classement[i].has_crashed ? "---" : at_formate, 
                classement[i].has_crashed ? "CRASH" : classement[i].nb_tours == nb_lap ? "END" : "RUN", 
                classement[i].has_crashed ? "CRASH" : diff_formate,
                classement[i].time_in_pit > 0 ? pit_time : "--:--");

        prev_at = classement[i].at;
    }
    printf("------------------------------------------------------------------------------------------------------------------\n");
    afficher_meilleurs_temps(data);
}


// Gère une séance de practice
void gerer_seance_practice(SharedData* data, int nb_voitures, char* course_name, int compteur) {
    pid_t pids[MAX_VOITURES];
    int tours_restants[MAX_VOITURES]; // Suivi des tours restants par voiture
    key_t sem_key = ftok("practice", 66); // Clé pour le sémaphore
    int semid = initialiser_semaphores(sem_key, 1); // 1 sémaphore pour synchroniser

    printf("\nEn attente du lancement de la séance d'entrainement, appuyez sur une touche pour continuer...\n");
    getchar(); // Attend une touche de l'utilisateur

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
                usleep(500000);
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
        usleep(250000); // afin d'éviter un affichage trop intense
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
void gerer_qualifications_classiques(SharedData* data, int nb_voitures, int nb_elimines, int duree_session, const char *course_name) {
   
    printf("\nEn attente du lancement de la séance de qualification, appuyez sur une touche pour continuer...\n");
    getchar(); // Attend une touche de l'utilisateur

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
            data->voitures[i].time_in_pit = 0;
            while (1) {
                verrouiller_semaphore(semid, 0);

                // Vérifiez si le temps de la session est terminé
                if (time(NULL) - debut_session > duree_session) {
                    data->voitures[i].time_in_pit = 1;
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
        usleep(250000); // Pause pour éviter trop de rafraîchissements
    }

    // Trier et marquer les voitures les plus lentes comme éliminées
    trier_voitures_qualifs(data->voitures, nb_voitures);

    // élimine les dernières voitures en fonction de la phase de qualif 
    for (int i = nb_voitures - nb_elimines; i < nb_voitures; i++) {
        data->voitures[i].in_pit = 1;
    }

    printf("\nSession terminée. Classement mis à jour.\n");
    afficher_classement_voiture_qualif(data->voitures, nb_voitures, data);
    enregistrer_qualif_classic(course_name, data->voitures, nb_voitures, data);

    for (int i = 0; i < nb_voitures - nb_elimines; i++) {
        data->voitures[i].bt = 0;             // Meilleur temps remis à zéro
        data->voitures[i].at = 0;             // Temps total remis à zéro
        data->voitures[i].nb_tours = 0;       // Nombre de tours remis à zéro
        data->voitures[i].has_crashed = 0;    // Indicateur de crash réinitialisé
        data->voitures[i].in_pit = 0;         // Indicateur aux stands réinitialisé
        data->voitures[i].has_pit = 0;        // Indicateur de nombre de passage au pit réinitialisé
        data->voitures[i].tours_depuis_pit = 0; // Indicateur de nombre de tours depuis le dernier pit réinitilisé
        data->voitures[i].time_in_pit = 0;        //  temps passer au pit remis à zéro
        data->voitures[i].tt = 0;             // temps total mis pour le tour remis à zéro
    }

    for (int i = nb_voitures - nb_elimines; i < nb_voitures; i++) {
        data->voitures[i].bt = ((i+1) * 60 * 1000) + ( (i+1) * 1000) + ( (i+1)* 10);             // Meilleur mis de manière à respecté la place (si i = 15 alors 15m.15s.15ms)
        data->voitures[i].at = 0;             // Temps total remis à zéro
        data->voitures[i].nb_tours = 0;       // Nombre de tours remis à zéro
        data->voitures[i].has_crashed = 0;    // Indicateur de crash réinitialisé
        data->voitures[i].has_pit = 0;        // Indicateur de nombre de passage au pit réinitialisé
        data->voitures[i].tours_depuis_pit = 0; // Indicateur de nombre de tours depuis le dernier pit réinitilisé
        data->voitures[i].time_in_pit = 0;        //  temps passer au pit remis à zéro
        data->voitures[i].tt = 0;             // temps total mis pour le tour remis à zéro
        data->voitures[i].s1 = 9999999;
        data->voitures[i].s1 = 9999999;
        data->voitures[i].s3 = 9999999;
    }

    data->best_all_s1 = 0;              // Meilleurs temps S1 remis à zéro
    data->best_all_s2 = 0;              // Meilleurs temps S2 remis à zéro
    data->best_all_s3 = 0;              // Meilleurs temps S3 remis à zéro
    data->best_all_time = 0;            // Meilleurs temps tour remis à zéro
    data->voiture_meilleur_s1 = -1;     // Numéro de voiture réinitialisé
    data->voiture_meilleur_s2 = -1;     // Numéro de voiture réinitialisé
    data->voiture_meilleur_s3 = -1;     // Numéro de voiture réinitialisé
    data->voiture_meilleur_temps = -1;
    detruire_semaphores(semid);
}

void attribuer_points_post_sprint(Pilote pilotes[], SharedData* data){
 const int SPRINT_POINTS[] = {8, 7, 6, 5, 4, 3, 2, 1};

 for (int i = 0; i < 8; i++) {
   for (int j =0; j < 20; j++) {
    if (data->voitures[i].number == pilotes[j].number) {
        pilotes[j].points += SPRINT_POINTS[i];
        break;
    }
   }
 }

}

void attribuer_points_post_course(Pilote pilotes[], SharedData* data) {
    const int COURSE_POINTS[] = {25, 20, 15, 10, 8, 6, 5, 3, 2, 1};
    int best_lap_bonus = 0;

    // Vérifier si la voiture ayant le meilleur tour est dans le top 10
    for (int i = 0; i < 10; i++) {
        if (data->voiture_meilleur_temps == data->voitures[i].number) {
            best_lap_bonus = 1; // La voiture est dans le top alors on va lui attribué un point en passant la valeur à 1
            break;
        }
    }

    // Attribuer les points de la course et du meilleur tour
    for (int i = 0; i < 10; i++) { // Parcourir les 10 premiers
        for (int j = 0; j < 20; j++) { // Trouver le pilote correspondant
            if (data->voitures[i].number == pilotes[j].number) {
                // Ajouter les points de la course et, si applicable, le bonus
                pilotes[j].points += COURSE_POINTS[i];
                if (data->voiture_meilleur_temps == data->voitures[i].number) {
                    pilotes[j].points += best_lap_bonus;
                }
                break; // une fois Pilote trouvé, passer à la voiture suivante pour ne pas perdre de temps à faire toute la boucle
            }
        }
    }
}

// gérer une séance de sprint
void gerer_seance_sprint(SharedData* data, int nb_voitures, Course course, Pilote pilotes[]) {
    printf("\nDébut de la séance sprint\n");
    // trier_voitures_qualifs(data->voitures, nb_voitures);
    reinitialiser_voitures(data->voitures, nb_voitures, data);

    // Calculer le nombre de tours nécessaires pour couvrir 100 km
    int tours_necessaires = (int)((100000.0 / course.km_lap)) + 1;

    afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_necessaires);
    printf("\nEn attente du lancement de la simulation du sprint, appuyez sur une touche pour continuer...\n");
    getchar(); // Attend une touche de l'utilisateur

    // Initialisation des variables nécessaires
    pid_t pids[MAX_VOITURES];
    key_t sem_key = ftok("sprint", 67); // Clé unique pour le sémaphore
    int semid = initialiser_semaphores(sem_key, 1); // Un seul sémaphore

    // Création des processus enfants
    for (int i = 0; i < nb_voitures; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Processus enfant : Gère une voiture
            srand(time(NULL) + getpid()); // Graine unique pour chaque processus

            while (1) {
                verrouiller_semaphore(semid, 0);

                // Vérifier si la voiture a terminé le nombre de tours nécessaires
                if (data->voitures[i].nb_tours >= tours_necessaires) {
                    deverrouiller_semaphore(semid, 0);
                    break; // Sortir si les tours nécessaires sont effectués
                }

                // Simuler un tour
                gerer_tour_course(&data->voitures[i], data, i, tours_necessaires);
                

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

        // Afficher le classement à intervalles réguliers
        afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_necessaires);
        usleep(250000); // Pause pour éviter trop de rafraîchissements
    }
    trier_voitures_course(data->voitures, nb_voitures);
    // Session terminée : Trier et afficher les résultats finaux
    printf("\nSéance sprint terminée. Classement final :\n");
    afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_necessaires);
    enregistrer_resultat_sprint(course.name, data->voitures, nb_voitures, data);
    attribuer_points_post_sprint(pilotes, data);
    reinitialiser_voitures(data->voitures, nb_voitures, data);
    detruire_semaphores(semid);
}


void gerer_seance_course(SharedData* data, int nb_voitures, Course course, Pilote pilotes[]) {
    printf("\nDébut de la séance sprint\n");
    // trier_voitures_qualifs(data->voitures, nb_voitures);
    reinitialiser_voitures(data->voitures, nb_voitures, data);

    int tours_total = course.nb_laps;

    afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_total);
    printf("\nEn attente du lancement, appuyez sur une touche pour continuer...\n");
    getchar(); // Attend une touche de l'utilisateur
    getchar(); // Attend une touche de l'utilisateur

    // Initialisation des variables nécessaires
    pid_t pids[MAX_VOITURES];
    key_t sem_key = ftok("race", 67); // Clé unique pour le sémaphore
    int semid = initialiser_semaphores(sem_key, 1); // Un seul sémaphore

   

    // Création des processus enfants
    for (int i = 0; i < nb_voitures; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Processus enfant : Gère une voiture
            srand(time(NULL) + getpid()); // Graine unique pour chaque processus

            while (1) {
                verrouiller_semaphore(semid, 0);

                // Vérifier si la voiture a terminé le nombre de tours nécessaires
                if (data->voitures[i].nb_tours >= tours_total) {
                    deverrouiller_semaphore(semid, 0);
                    break; // Sortir si les tours nécessaires sont effectués
                }

                // Simuler un tour
                gerer_tour_course(&data->voitures[i], data, i, tours_total);
                

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

        // Afficher le classement à intervalles réguliers
        afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_total);
        usleep(250000); // Pause pour éviter trop de rafraîchissements
    }

    // Session terminée : Trier et afficher les résultats finaux
    printf("\nSéance course terminée. Classement final :\n");
    trier_voitures_course(data->voitures, nb_voitures);
    afficher_classement_voiture_sprint(data->voitures, nb_voitures, data, tours_total);
    enregistrer_resultat_course(course.name, data->voitures, nb_voitures, data);
    attribuer_points_post_course(pilotes, data);
    detruire_semaphores(semid);
}

// Permet de Gérer la simulation d'un Weekend qu'il soit Spécial ou Classique
void gerer_weekend(WeekendType type, SharedData* data, int nb_voitures, Course course, Pilote pilotes[]) {
    system("clear");
    printf("Début du weekend de course (%s)\n", type == CLASSIC ? "CLASSIQUE" : "SPECIAL");
    int compteur_practice = 0;
    char choix;

    // Jour 1 : Vendredi - Séances d'essais
    printf("Souhaitez-vous simuler les séances d'essais ? [o/n]\n");
    scanf(" %c", &choix);

    switch (choix) {
        case 'o':
        case 'O':
            printf("Voulez-vous lancer la Séance P1 ? [o/n]\n");
            scanf(" %c", &choix);
            switch (choix) {
                case 'o':
                case 'O':
                    compteur_practice = 1;
                    gerer_seance_practice(data, nb_voitures, course.name, compteur_practice);

                    printf("Voulez-vous lancer la Séance P2 ? [o/n]\n");
                    scanf(" %c", &choix);
                    switch (choix) {
                        case 'o':
                        case 'O':
                            compteur_practice = 2;
                            gerer_seance_practice(data, nb_voitures, course.name, compteur_practice);

                            printf("Voulez-vous lancer la Séance P3 ? [o/n]\n");
                            scanf(" %c", &choix);
                            switch (choix) {
                                case 'o':
                                case 'O':
                                    compteur_practice = 3;
                                    gerer_seance_practice(data, nb_voitures, course.name, compteur_practice);
                                    break;
                                default:
                                    printf("La séance P3 n'a pas été exécutée.\n");
                            }
                            break;
                        default:
                            printf("La séance P2 (et P3) n'ont pas été exécutées.\n");
                    }
                    break;
                default:
                    printf("Aucune séance d'essais (Practice) n'a été exécutée.\n");
            }
            break;
        default:
            printf("Les séances d'essais ont été ignorées.\n");
    }

    // Jour 2 : Samedi - Qualifications
    printf("\n--- Jour 2 : Samedi ---\n");
    if (type == SPECIAL) {
        printf("Souhaitez-vous simuler les qualifications pour le Sprint ? [o/n]\n");
        scanf(" %c", &choix);

        switch (choix) {
            case 'o':
            case 'O':
                printf("Voulez-vous lancer la qualification Sprint Q1 ? [o/n]\n");
                scanf(" %c", &choix);
                switch (choix) {
                    case 'o':
                    case 'O':
                        gerer_qualifications_classiques(data, nb_voitures, 5, Q1_SPRINT, course.name);

                        printf("Voulez-vous lancer la qualification Sprint Q2 ? [o/n]\n");
                        scanf(" %c", &choix);
                        switch (choix) {
                            case 'o':
                            case 'O':
                                gerer_qualifications_classiques(data, nb_voitures, 10, Q2_SPRINT, course.name);

                                printf("Voulez-vous lancer la qualification Sprint Q3 ? [o/n]\n");
                                scanf(" %c", &choix);
                                switch (choix) {
                                    case 'o':
                                    case 'O':
                                        gerer_qualifications_classiques(data, nb_voitures, 0, Q3_SPRINT, course.name);
                                        break;
                                    default:
                                        printf("La qualification Sprint Q3 n'a pas été exécutée.\n");
                                }
                                break;
                            default:
                                printf("La qualification Sprint Q2 (et Q3) n'ont pas été exécutées.\n");
                        }
                        break;
                    default:
                        printf("Aucune qualification Sprint (Q1, Q2, Q3) n'a été exécutée.\n");
                }
                break;
            default:
                printf("Les qualifications pour le Sprint ont été ignorées.\n");
        }

        printf("\nCourse Sprint (~100 km)\n");
        gerer_seance_sprint(data, nb_voitures, course, pilotes);
    }

    // Qualifications pour la course principale
    printf("\nSouhaitez-vous simuler les qualifications pour la course principale ? [o/n]\n");
    scanf(" %c", &choix);

    switch (choix) {
        case 'o':
        case 'O':
            printf("Voulez-vous lancer la qualification Q1 ? [o/n]\n");
            scanf(" %c", &choix);
            switch (choix) {
                case 'o':
                case 'O':
                    gerer_qualifications_classiques(data, nb_voitures, 5, Q1_CLASSIC, course.name);

                    printf("Voulez-vous lancer la qualification Q2 ? [o/n]\n");
                    scanf(" %c", &choix);
                    switch (choix) {
                        case 'o':
                        case 'O':
                            gerer_qualifications_classiques(data, nb_voitures, 10, Q2_CLASSIC, course.name);

                            printf("Voulez-vous lancer la qualification Q3 ? [o/n]\n");
                            scanf(" %c", &choix);
                            switch (choix) {
                                case 'o':
                                case 'O':
                                    gerer_qualifications_classiques(data, nb_voitures, 0, Q3_CLASSIC, course.name);
                                    break;
                                default:
                                    printf("La qualification Q3 n'a pas été exécutée.\n");
                            }
                            break;
                        default:
                            printf("La qualification Q2 (et Q3) n'ont pas été exécutées.\n");
                    }
                    break;
                default:
                    printf("Aucune qualification (Q1, Q2, Q3) n'a été exécutée.\n");
            }
            break;
        default:
            printf("Les qualifications pour la course principale ont été ignorées.\n");
    }

    // Jour 3 : Dimanche - Course principale
    printf("\n--- Jour 3 : Dimanche ---\n");
    printf("Course principale\n");
    gerer_seance_course(data, nb_voitures, course, pilotes);

    printf("Weekend terminé.\n");
}

