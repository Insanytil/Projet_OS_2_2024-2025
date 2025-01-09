#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib/sauvegarde.h"
#include "lib/voitures.h"


#define BUFFER_SIZE 256

void sauvegarderPilotesCSV(Pilote pilotes[], int taille, const char* filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier pour la sauvegarde des pilotes.");
        return;
    }

    char buffer[256];
    for(int i = 0; i < taille; i++) {
        snprintf(buffer, sizeof(buffer), "%s,%s,%d,%s,%s,%d\n",
            pilotes[i].first_name,
            pilotes[i].last_name,
            pilotes[i].number,
            pilotes[i].team_name,
            pilotes[i].nickname,
            pilotes[i].points);
        write(fd, buffer, strlen(buffer));
    }

    close(fd);
    printf("Sauvegarde des pilotes terminée dans '%s'.\n", filename);

}

char* recupererCheminSauvegarde(const char* chemin) {
    DIR* dir = opendir(chemin);
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        return NULL;
    }

    struct dirent* entry;
    char** fichiers = malloc(100 * sizeof(char*)); // Allocation dynamique pour un maximum de 100 fichiers
    int count = 0;

    // Parcourir le répertoire
    while ((entry = readdir(dir)) != NULL) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", chemin, entry->d_name);

        struct stat entry_stat;
        if (stat(fullpath, &entry_stat) == 0 && S_ISREG(entry_stat.st_mode)) { // Vérifier si c'est un fichier régulier
            if (strstr(entry->d_name, ".csv")) { // Vérifier si le fichier a l'extension .csv
                fichiers[count] = strdup(entry->d_name);
                count++;
            }
        }
    }

    closedir(dir);

    if (count == 0) {
        printf("Aucune sauvegarde trouvée dans '%s'.\n", chemin);
        free(fichiers);
        return NULL;
    }

    printf("Sauvegardes disponibles :\n");
    for (int i = 0; i < count; i++) {
        printf("  %d. %s\n", i + 1, fichiers[i]);
    }

    // Demander à l'utilisateur de sélectionner un fichier
    int choix = 0;
    do {
        printf("Sélectionnez une sauvegarde (1-%d) : ", count);
        scanf("%d", &choix);
    } while (choix < 1 || choix > count);

    // Construire le chemin complet pour le fichier sélectionné
    char* chemin_complet = malloc(1024);
    snprintf(chemin_complet, 1024, "%s/%s", chemin, fichiers[choix - 1]);

    for (int i = 0; i < count; i++) {
        free(fichiers[i]);
    }
    free(fichiers);

    return chemin_complet;
}




void enregistrer_qualif_classic(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/courses/%s.csv", nom_course);

    // Ouvrir le fichier en mode écriture (création si inexistant, ajout sinon)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Ajouter une ligne de séparation
    const char *separateur = "---------------------\n";
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur");
        close(fd);
        return;
    }

    // Ajouter la date, l'heure et le numéro de practice sur une seule ligne
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char datetime[256];
    snprintf(datetime, sizeof(datetime), "Date et heure : %02d/%02d/%04d %02d:%02d:%02d - Classement Qualif course\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, // Date (jour/mois/année)
             t->tm_hour, t->tm_min, t->tm_sec);            // Heure (heure:minute:seconde)

    if (write(fd, datetime, strlen(datetime)) < 0) {
        perror("Erreur lors de l'écriture de la date et heure avec Practice n°");
        close(fd);
        return;
    }

    // Écriture des données des voitures
    char ligne[256];
    for (int i = 0; i < nb_voitures; i++) {
        if (voitures[i].bt > 0) {
            snprintf(ligne, sizeof(ligne), "%s,%d,%s,%d,%02d:%02d.%03d\n",
                     nom_course,
                     i + 1,
                     voitures[i].code_pilote,
                     voitures[i].number,
                     voitures[i].bt / 60000,               // Minutes
                     (voitures[i].bt % 60000) / 1000,      // Secondes
                     voitures[i].bt % 1000);               // Millisecondes
            if (write(fd, ligne, strlen(ligne)) < 0) {
                perror("Erreur lors de l'écriture des données");
                close(fd);
                return;
            }
        }
    }

    // Ajouter les meilleurs temps par secteur (S1, S2, S3) et le meilleur temps global
    const char *header_meilleurs_temps = "\nMeilleurs temps par secteur et meilleur tour global :\n";
    if (write(fd, header_meilleurs_temps, strlen(header_meilleurs_temps)) < 0) {
        perror("Erreur lors de l'écriture de l'en-tête des meilleurs temps");
        close(fd);
        return;
    }

    snprintf(ligne, sizeof(ligne),
             "Meilleur S1 : %02d:%02d.%03d par %d\n"
             "Meilleur S2 : %02d:%02d.%03d par %d\n"
             "Meilleur S3 : %02d:%02d.%03d par %d\n"
             "Meilleur tour global : %02d:%02d.%03d par %d\n",
             data->best_all_s1/ 60000,
             (data->best_all_s1/ 1000) % 60,
             data->best_all_s1% 1000,
             data->voiture_meilleur_s1,  // Code pilote pour S1
             data->best_all_s2 / 60000,
             (data->best_all_s2 / 1000) % 60,
             data->best_all_s2 % 1000,
             data->voiture_meilleur_s2,  // Code pilote pour S2
             data->best_all_s3 / 60000,
             (data->best_all_s3 / 1000) % 60,
             data->best_all_s3 % 1000,
             data->voiture_meilleur_s3,  // Code pilote pour S3
             data->best_all_time / 60000,
             (data->best_all_time / 1000) % 60,
             data->best_all_time % 1000,
             data->voiture_meilleur_temps); // Code pilote pour le meilleur tour global

    if (write(fd, ligne, strlen(ligne)) < 0) {
        perror("Erreur lors de l'écriture des meilleurs temps");
        close(fd);
        return;
    }

    // Ajouter une ligne de séparation pour conclure
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur final");
        close(fd);
        return;
    }

    // Fermer le fichier
    if (close(fd) < 0) {
        perror("Erreur lors de la fermeture du fichier");
    } else {
        printf("Résultats sauvegardés dans %s\n", filepath);
    }
}


void enregistrer_qualif_sprint(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/courses/%s.csv", nom_course);

    // Ouvrir le fichier en mode écriture (création si inexistant, ajout sinon)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier durant la sauvegarde du classement de la qualification du sprint");
        return;
    }

    // Ajouter une ligne de séparation
    const char *separateur = "---------------------\n";
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur durant la sauvegarde du classement de la qualification");
        close(fd);
        return;
    }

    // Ajouter la date, l'heure et le numéro de practice sur une seule ligne
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char datetime[256];
    snprintf(datetime, sizeof(datetime), "Date et heure : %02d/%02d/%04d %02d:%02d:%02d - Classement Qualif Sprint\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, // Date (jour/mois/année)
             t->tm_hour, t->tm_min, t->tm_sec);            // Heure (heure:minute:seconde)

    if (write(fd, datetime, strlen(datetime)) < 0) {
        perror("Erreur lors de l'écriture de la date et heure durant la sauvegarde du classement de la qualification");
        close(fd);
        return;
    }

    // Écriture des données des voitures
    char ligne[256];
    for (int i = 0; i < nb_voitures; i++) {
        if (voitures[i].bt > 0) {
            snprintf(ligne, sizeof(ligne), "%s,%d,%s,%d,%02d:%02d.%03d\n",
                     nom_course,
                     i + 1,
                     voitures[i].code_pilote,
                     voitures[i].number,
                     voitures[i].bt / 60000,               // Minutes
                     (voitures[i].bt % 60000) / 1000,      // Secondes
                     voitures[i].bt % 1000);               // Millisecondes
            if (write(fd, ligne, strlen(ligne)) < 0) {
                perror("Erreur lors de l'écriture des données");
                close(fd);
                return;
            }
        }
    }

    // Ajouter les meilleurs temps par secteur (S1, S2, S3) et le meilleur temps global
    const char *header_meilleurs_temps = "\nMeilleurs temps par secteur et meilleur tour global :\n";
    if (write(fd, header_meilleurs_temps, strlen(header_meilleurs_temps)) < 0) {
        perror("Erreur lors de l'écriture de l'en-tête des meilleurs temps durant la sauvegarde du classement de la qualification");
        close(fd);
        return;
    }

    snprintf(ligne, sizeof(ligne),
             "Meilleur S1 : %02d:%02d.%03d par %d\n"
             "Meilleur S2 : %02d:%02d.%03d par %d\n"
             "Meilleur S3 : %02d:%02d.%03d par %d\n"
             "Meilleur tour global : %02d:%02d.%03d par %d\n",
             data->best_all_s1/ 60000,
             (data->best_all_s1/ 1000) % 60,
             data->best_all_s1% 1000,
             data->voiture_meilleur_s1,  // Code pilote pour S1
             data->best_all_s2 / 60000,
             (data->best_all_s2 / 1000) % 60,
             data->best_all_s2 % 1000,
             data->voiture_meilleur_s2,  // Code pilote pour S2
             data->best_all_s3 / 60000,
             (data->best_all_s3 / 1000) % 60,
             data->best_all_s3 % 1000,
             data->voiture_meilleur_s3,  // Code pilote pour S3
             data->best_all_time / 60000,
             (data->best_all_time / 1000) % 60,
             data->best_all_time % 1000,
             data->voiture_meilleur_temps); // Code pilote pour le meilleur tour global

    if (write(fd, ligne, strlen(ligne)) < 0) {
        perror("Erreur lors de l'écriture des meilleurs temps durant la sauvegarde du classement de la qualification");
        close(fd);
        return;
    }

    // Ajouter une ligne de séparation pour conclure
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur final durant la sauvegarde du classement de la qualification");
        close(fd);
        return;
    }

    // Fermer le fichier
    if (close(fd) < 0) {
        perror("Erreur lors de la fermeture du fichier durant la sauvegarde du classement de la qualification");
    } else {
        printf("Résultats sauvegardés dans %s\n", filepath);
    }
}


void enregistrer_practice(const char *nom_course, Voiture voitures[], int nb_voitures, int compteur, SharedData* data) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/courses/%s.csv", nom_course);

    // Ouvrir le fichier en mode écriture (création si inexistant, ajout sinon)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier dans la sauvegarde de la séance d'entrainement");
        return;
    }

    // Ajouter une ligne de séparation
    const char *separateur = "---------------------\n";
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur dans la sauvegarde de la séance d'entrainement");
        close(fd);
        return;
    }

    // Ajouter la date, l'heure et le numéro de practice sur une seule ligne
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char datetime[256];
    snprintf(datetime, sizeof(datetime), "Date et heure : %02d/%02d/%04d %02d:%02d:%02d - Practice n°%d\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, // Date (jour/mois/année)
             t->tm_hour, t->tm_min, t->tm_sec,             // Heure (heure:minute:seconde)
             compteur);

    if (write(fd, datetime, strlen(datetime)) < 0) {
        perror("Erreur lors de l'écriture de la date et heure dans la sauvegarde de la séance d'entrainement");
        close(fd);
        return;
    }

    // Écriture des données
    char ligne[256];
    for (int i = 0; i < nb_voitures; i++) {
        if (voitures[i].bt > 0) {
            snprintf(ligne, sizeof(ligne), "%s,%s,%d,%02d:%02d.%03d\n",
                     nom_course,
                     voitures[i].code_pilote,
                     voitures[i].number,
                     voitures[i].bt / 60000,               // Minutes
                     (voitures[i].bt % 60000) / 1000,      // Secondes
                     voitures[i].bt % 1000);               // Millisecondes
            if (write(fd, ligne, strlen(ligne)) < 0) {
                perror("Erreur lors de l'écriture des données dans la sauvegarde de la séance d'entrainement");
                close(fd);
                return;
            }
        }
    }

    // Ajouter les meilleurs temps par secteur (S1, S2, S3) et le meilleur temps global
    const char *header_meilleurs_temps = "\nMeilleurs temps par secteur et meilleur tour global :\n";
    if (write(fd, header_meilleurs_temps, strlen(header_meilleurs_temps)) < 0) {
        perror("Erreur lors de l'écriture de l'en-tête des meilleurs temps dans la sauvegarde de la séance d'entrainement");
        close(fd);
        return;
    }

    snprintf(ligne, sizeof(ligne),
             "Meilleur S1 : %02d:%02d.%03d par %d\n"
             "Meilleur S2 : %02d:%02d.%03d par %d\n"
             "Meilleur S3 : %02d:%02d.%03d par %d\n"
             "Meilleur tour global : %02d:%02d.%03d par %d\n",
             data->best_all_s1/ 60000,
             (data->best_all_s1/ 1000) % 60,
             data->best_all_s1% 1000,
             data->voiture_meilleur_s1,  // Code pilote pour S1
             data->best_all_s2 / 60000,
             (data->best_all_s2 / 1000) % 60,
             data->best_all_s2 % 1000,
             data->voiture_meilleur_s2,  // Code pilote pour S2
             data->best_all_s3 / 60000,
             (data->best_all_s3 / 1000) % 60,
             data->best_all_s3 % 1000,
             data->voiture_meilleur_s3,  // Code pilote pour S3
             data->best_all_time / 60000,
             (data->best_all_time / 1000) % 60,
             data->best_all_time % 1000,
             data->voiture_meilleur_temps); // Code pilote pour le meilleur tour global

    if (write(fd, ligne, strlen(ligne)) < 0) {
        perror("Erreur lors de l'écriture des meilleurs temps dans la sauvegarde de la séance d'entrainement");
        close(fd);
        return;
    }

    // Ajouter une ligne de séparation pour conclure
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur final dans la sauvegarde de la séance d'entrainement");
        close(fd);
        return;
    }


    // Fermer le fichier
    if (close(fd) < 0) {
        perror("Erreur lors de la fermeture du fichier dans la sauvegarde de la séance d'entrainement");
    } else {
        printf("Résultats sauvegardés dans %s\n", filepath);
    }
}

void enregistrer_resultat_sprint(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/courses/%s.csv", nom_course);

    // Ouvrir le fichier en mode écriture (création si inexistant, ajout sinon)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier dans la sauvegarde du classement du Sprint");
        return;
    }

    // Ajouter une ligne de séparation
    const char *separateur = "---------------------\n";
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur dans la sauvegarde du classement du Sprint");
        close(fd);
        return;
    }

    // Ajouter la date, l'heure  sur une seule ligne
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char datetime[256];
    snprintf(datetime, sizeof(datetime), "Date et heure : %02d/%02d/%04d %02d:%02d:%02d - Classement du Sprint\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, // Date (jour/mois/année)
             t->tm_hour, t->tm_min, t->tm_sec);            // Heure (heure:minute:seconde)

    if (write(fd, datetime, strlen(datetime)) < 0) {
        perror("Erreur lors de l'écriture de la date et heure dans la sauvegarde du classement du Sprint");
        close(fd);
        return;
    }

    // Écriture des données des voitures
    char ligne[256];
    for (int i = 0; i < nb_voitures; i++) {
        if (voitures[i].bt > 0) {
            snprintf(ligne, sizeof(ligne), "%s,%d,%s,%d,%02d:%02d.%03d\n",
                     nom_course,
                     i + 1,
                     voitures[i].code_pilote,
                     voitures[i].number,
                     voitures[i].bt / 60000,               // Minutes
                     (voitures[i].bt % 60000) / 1000,      // Secondes
                     voitures[i].bt % 1000);               // Millisecondes
            if (write(fd, ligne, strlen(ligne)) < 0) {
                perror("Erreur lors de l'écriture des données dans la sauvegarde du classement du Sprint");
                close(fd);
                return;
            }
        }
    }

    // Ajouter les meilleurs temps par secteur (S1, S2, S3) et le meilleur temps global
    const char *header_meilleurs_temps = "\nMeilleurs temps par secteur et meilleur tour global du Sprint:\n";
    if (write(fd, header_meilleurs_temps, strlen(header_meilleurs_temps)) < 0) {
        perror("Erreur lors de l'écriture de l'en-tête des meilleurs temps dans la sauvegarde du classement du Sprint");
        close(fd);
        return;
    }

    snprintf(ligne, sizeof(ligne),
             "Meilleur S1 : %02d:%02d.%03d par %d\n"
             "Meilleur S2 : %02d:%02d.%03d par %d\n"
             "Meilleur S3 : %02d:%02d.%03d par %d\n"
             "Meilleur tour global : %02d:%02d.%03d par %d\n",
             data->best_all_s1/ 60000,
             (data->best_all_s1/ 1000) % 60,
             data->best_all_s1% 1000,
             data->voiture_meilleur_s1,  // Code pilote pour S1
             data->best_all_s2 / 60000,
             (data->best_all_s2 / 1000) % 60,
             data->best_all_s2 % 1000,
             data->voiture_meilleur_s2,  // Code pilote pour S2
             data->best_all_s3 / 60000,
             (data->best_all_s3 / 1000) % 60,
             data->best_all_s3 % 1000,
             data->voiture_meilleur_s3,  // Code pilote pour S3
             data->best_all_time / 60000,
             (data->best_all_time / 1000) % 60,
             data->best_all_time % 1000,
             data->voiture_meilleur_temps); // Code pilote pour le meilleur tour global

    if (write(fd, ligne, strlen(ligne)) < 0) {
        perror("Erreur lors de l'écriture des meilleurs temps dans la sauvegarde du classement du Sprint");
        close(fd);
        return;
    }

    // Ajouter une ligne de séparation pour conclure
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur final dans la sauvegarde du classement du Sprint");
        close(fd);
        return;
    }

    // Fermer le fichier
    if (close(fd) < 0) {
        perror("Erreur lors de la fermeture du fichier dans la sauvegarde du classement du Sprint");
    } else {
        printf("Résultats du Sprint sauvegardés dans %s\n", filepath);
    }
}

void enregistrer_resultat_course(const char *nom_course, Voiture voitures[], int nb_voitures, SharedData* data) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/courses/%s.csv", nom_course);

    // Ouvrir le fichier en mode écriture (création si inexistant, ajout sinon)
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier dans la sauvegarde du classement de la course finale");
        return;
    }

    // Ajouter une ligne de séparation
    const char *separateur = "---------------------\n";
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur dans la sauvegarde du classement de la course finale");
        close(fd);
        return;
    }

    // Ajouter la date, l'heure  sur une seule ligne
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char datetime[256];
    snprintf(datetime, sizeof(datetime), "Date et heure : %02d/%02d/%04d %02d:%02d:%02d - Classement de la course Finale\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, // Date (jour/mois/année)
             t->tm_hour, t->tm_min, t->tm_sec);            // Heure (heure:minute:seconde)

    if (write(fd, datetime, strlen(datetime)) < 0) {
        perror("Erreur lors de l'écriture de la date et heure dans la sauvegarde du classement de la course finale");
        close(fd);
        return;
    }

    // Écriture des données des voitures
    char ligne[256];
    for (int i = 0; i < nb_voitures; i++) {
        if (voitures[i].bt > 0) {
            snprintf(ligne, sizeof(ligne), "%s,%d,%s,%d,%02d:%02d.%03d\n",
                     nom_course,
                     i + 1,
                     voitures[i].code_pilote,
                     voitures[i].number,
                     voitures[i].bt / 60000,               // Minutes
                     (voitures[i].bt % 60000) / 1000,      // Secondes
                     voitures[i].bt % 1000);               // Millisecondes
            if (write(fd, ligne, strlen(ligne)) < 0) {
                perror("Erreur lors de l'écriture des données dans la sauvegarde du classement de la course finale");
                close(fd);
                return;
            }
        }
    }

    // Ajouter les meilleurs temps par secteur (S1, S2, S3) et le meilleur temps global
    const char *header_meilleurs_temps = "\nMeilleurs temps par secteur et meilleur tour global de la course finale:\n";
    if (write(fd, header_meilleurs_temps, strlen(header_meilleurs_temps)) < 0) {
        perror("Erreur lors de l'écriture de l'en-tête des meilleurs temps dans la sauvegarde du classement de la course finale");
        close(fd);
        return;
    }

    snprintf(ligne, sizeof(ligne),
             "Meilleur S1 : %02d:%02d.%03d par %d\n"
             "Meilleur S2 : %02d:%02d.%03d par %d\n"
             "Meilleur S3 : %02d:%02d.%03d par %d\n"
             "Meilleur tour global : %02d:%02d.%03d par %d\n",
             data->best_all_s1/ 60000,
             (data->best_all_s1/ 1000) % 60,
             data->best_all_s1% 1000,
             data->voiture_meilleur_s1,  // Code pilote pour S1
             data->best_all_s2 / 60000,
             (data->best_all_s2 / 1000) % 60,
             data->best_all_s2 % 1000,
             data->voiture_meilleur_s2,  // Code pilote pour S2
             data->best_all_s3 / 60000,
             (data->best_all_s3 / 1000) % 60,
             data->best_all_s3 % 1000,
             data->voiture_meilleur_s3,  // Code pilote pour S3
             data->best_all_time / 60000,
             (data->best_all_time / 1000) % 60,
             data->best_all_time % 1000,
             data->voiture_meilleur_temps); // Code pilote pour le meilleur tour global

    if (write(fd, ligne, strlen(ligne)) < 0) {
        perror("Erreur lors de l'écriture des meilleurs temps dans la sauvegarde du classement de la course finale");
        close(fd);
        return;
    }

    // Ajouter une ligne de séparation pour conclure
    if (write(fd, separateur, strlen(separateur)) < 0) {
        perror("Erreur lors de l'écriture du séparateur final dans la sauvegarde du classement de la course finale");
        close(fd);
        return;
    }

    // Fermer le fichier
    if (close(fd) < 0) {
        perror("Erreur lors de la fermeture du fichier dans la sauvegarde du classement de la course finale");
    } else {
        printf("Résultats de la course finale sauvegardés dans %s\n", filepath);
    }
}
