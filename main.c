#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "lib/pilotes.h"
#include "lib/courses.h"
#include "lib/sauvegarde.h"
#include "lib/voitures.h"

#define PATH "data/saves/"

int main(int argc, char **argv) {
    int choice = 0;
    Pilote pilotes[20];
    Course courses[25];
    char* save = NULL;

    // Menu principal
    while (1) {
        printf("\nBienvenue dans le simulateur du championnat de Formule 1 2024-2025 :\n");
        printf("    1. Créer un nouveau championnat\n");
        printf("    2. Charger un championnat\n");
        printf("    3. Quitter\n");
        printf("        Que souhaitez-vous faire ? : ");
        scanf("%d", &choice);

        // Gestion des choix
        switch (choice) {
            case 1:
                system("clear");
                printf("Option 1 sélectionnée : Création d'un nouveau championnat.\n");
                initPilotes(pilotes, "data/pilotes.csv");
                initCourses(courses, "data/courses.csv");
                break;

            case 2: {
                system("clear");
                printf("Option 2 sélectionnée : Chargement d'un championnat existant.\n");
                char* chemin_sauvegarde = recupererCheminSauvegarde(PATH);
                
                if (chemin_sauvegarde != NULL) {
                    printf("Sauvegarde sélectionnée : %s\n", chemin_sauvegarde);

                    // Libérer `save` si nécessaire et mettre à jour
                    if (save != NULL) {
                        free(save);
                    }
                    save = strdup(chemin_sauvegarde);

                    initPilotes(pilotes, save);
                    initCourses(courses, "data/courses.csv");

                    free(chemin_sauvegarde);
                } else {
                    printf("Aucune sauvegarde n'a été sélectionnée.\n");
                }
                break;
            }

            case 3:
                printf("Option 3 sélectionnée : Quitter le programme. À bientôt !\n");
                return 0;

            default:
                system("clear");
                printf("Option invalide. Veuillez choisir 1, 2 ou 3.\n");
                continue;
        }

        // Sortir de la boucle après un choix valide
        if (choice == 1 || choice == 2) {
            break;
        }
    }

    // Menu secondaire
    while (1) {
        printf("\nQue souhaitez-vous faire :\n");
        printf("    1 - Afficher le Classement\n");
        printf("    2 - Lancer une course\n");
        printf("    3 - Sauvegarder\n");
        printf("    4 - Quitter\n");
        printf("        Votre choix : ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                system("clear");
                afficherClassement(pilotes, 20);
                break;

            case 2:
                simulerCourse(courses, 25, pilotes, 20);
                break;

            case 3: {
                // Gestion de la sauvegarde
                char nom_sauvegarde[256];
                printf("\nEntrez le nom de la sauvegarde (sans extension) : ");
                scanf("%s", nom_sauvegarde);

                char chemin_complet[512];
                snprintf(chemin_complet, sizeof(chemin_complet), "%s%s.csv", PATH, nom_sauvegarde);

                sauvegarderPilotesCSV(pilotes, 20, chemin_complet);

                if (save != NULL) {
                    free(save);
                }
                save = strdup(chemin_complet);

                printf("Sauvegarde effectuée dans : %s\n", save);
                break;
            }

            case 4:
                printf("\nOption 4 sélectionnée : Quitter le programme. Merci d'avoir joué !\n");
                libererMemoirePilote(pilotes, 20);
                libererMemoireCourses(courses, 25);
                if (save != NULL) {
                    free(save);
                }
                return 0;

            default:
                printf("Option invalide. Veuillez choisir 1, 2, 3 ou 4.\n");
                break;
        }
    }

    return 0;
}
