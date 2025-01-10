
# Projet_OS_2_2024-2025
# **Simulation de Week-end de Formule 1**

## **Description**
Ce projet est une simulation d'un week-end de Formule 1 en langage C, conçu pour être exécuté sur une machine Linux. Il permet de gérer des séances de practice, des courses, et le classement des pilotes tout en mettant en œuvre des concepts avancés de programmation système, tels que :
- La **mémoire partagée** (System V IPC).
- La **synchronisation via sémaphores**.
- La gestion de **processus multiples** avec `fork()`.
- L’utilisation de **fonctions bas niveau** (`open`, `read`, `write`).

---

## **Fonctionnalités**
- Création ou chargement de championnats à partir de fichiers CSV.
- Simulation de week-ends de Formule 1 comprenant :
  - **Practice** : Simulation des tours pour chaque voiture avec classement.
  - **Courses** : Gestion complète des courses avec données partagées.
- Gestion et sauvegarde des données de championnat (pilotes, points, courses, etc.).
- Affichage des classements en temps réel et finaux.
- Utilisation efficace des mécanismes de synchronisation pour éviter les conflits d'accès.

---

## **Prérequis**
Avant de compiler et d'exécuter ce projet, assurez-vous d'avoir :
- Un système Linux (Ubuntu, Debian, ou autre distribution compatible).
- Les outils suivants installés :
  - **GCC** (GNU Compiler Collection).
  - Les bibliothèques nécessaires pour la gestion de la mémoire partagée et des sémaphores (`sys/types.h`, `sys/sem.h`, `sys/shm.h`).

---

## **Installation**
1. **Cloner le dépôt** :
   ```bash
   git clone <URL_DU_DEPOT>
   cd <NOM_DU_DEPOT>
    ```
2. **Compiler le programme** :
   ```gcc -o championship main.c pilotes.c courses.c voitures.c sauvegarde.c weekend.c -lrt```
   - Explications des options :
     - ```lrt``` : Liens pour les fonctions de gestion de temps requises pour certains mécanismes bas niveau (pas sur de sa nécessité mais au cas où ça fait jamais de mal).

## **Utilisation**
1. **Lancer le programme**:
   ```/simulation```
2. **Menu Principal**:
   - Créer un nouveau championnat : Charge les données de pilotes.csv et courses.csv.
   - Charger une sauvegarde existante: Charge un état sauvegardé à partir d'un fichier.
   - Quitter le programme.
3. **Menu secondaire** (après avoir lancé un championnat)
    - Afficher le classement actuel.
    - Lancer un weekend de course (redirection vers un menu et le choix de la course).
    - Sauvegarder le championnat.
    - Retourner au menu principal.
## **Structure du projet**
### **Code source**
   - **main.c** : Point d'entrée du programme.
   - **pilotes.c / pilotes.h** : Gestion des pilotes (initialisation, classement).
   - **courses.c / courses.h** : Gestion des courses (initialisation, résultat).
   - **voitures.c / voitures.h** : Gestion et simulation des voitures et de leurs tours.
   - **sauvegarde.c / sauvegarde.h** : Gestion des sauvegardes
   - **weekend.c / weekend.h** : Coordination des étapes d'un week-end de course.
### **Données** :
    - **pilotes.csv** : Données des pilotes (nom, prénoms, numéro...).
    - **courses.csv** : Données des courses (nom, distance, pays...).

## **Conception technique** :
### **Mémoire partagée** :
    - Utilisée pour stocker les données des voitures lors des séances.
### **Sémaphore system v** :
    - Utilisé pour vérouiller/dévérouiller l'accès à la mémoire partagée.
### **Processus multiples** : 
   - Chaque voiture est simulée par un processus.
   - Le processus parent surveille les enfants et affiche régulièrement le classement.

### **AUTEUR**
    - VERVAEREN Lucien

### **LICENCE** :
   - Ce projet est sous licence MIT. Vous êtes libres de l'utiliser, le modifier, et le redistribuer sous les conditions de cette licence.
