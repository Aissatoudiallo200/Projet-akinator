#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include "GfxLib.h"

#include "BmpLib.h"

#include "ESLib.h"



#define LargeurFenetre 1080

#define HauteurFenetre 1080



typedef enum {ATTENTE, QUESTION, REJOUER, FIN, FIN_JEU, SAISIE_PAYS, SAISIE_QUESTION} Etat;



Etat etat = ATTENTE;

char questionTexte[256];

char chemin[256] = "";

int questionCount = 0;

FILE *f_questions;

FILE *f_reponses;

DonneesImageRGB *akinatorImage = NULL;



typedef struct Noeud {

    char question[256];

    struct Noeud *oui;

    struct Noeud *non;

} Noeud;



Noeud *racine = NULL;



void gestionEvenement(EvenementGfx evenement);

Noeud* creerNoeud(char *question);

void construireArbre(Noeud *racine, FILE *f_questions);

void obtenirQuestionSuivante(Noeud *racine, char *chemin, char *questionTexte);

void sauvegarderPays(const char *pays);

void sauvegarderQuestion(const char *chemin, const char *question);



int main(int argc, char **argv) {

    initialiseGfx(argc, argv);

    prepareFenetreGraphique("Projet Pays", LargeurFenetre, HauteurFenetre);



    f_questions = fopen("questions.txt", "r+");

    f_reponses = fopen("pays.txt", "a+");



    if (f_questions == NULL) {

        perror("Erreur d'ouverture du fichier questions.txt");

        return 1;

    }



    if (f_reponses == NULL) {

        perror("Erreur d'ouverture du fichier pays.txt");

        fclose(f_questions);

        return 1;

    }



    // Initialiser l'arbre de décision

    racine = creerNoeud("Ce pays est-il membre de l'Union Européenne ?");

    construireArbre(racine, f_questions);



    // Obtenir la première question

    obtenirQuestionSuivante(racine, chemin, questionTexte);

    etat = QUESTION;



    akinatorImage = lisBMPRGB("akinator.bmp");

    if (akinatorImage == NULL) {

        perror("Erreur de chargement de l'image akinator.bmp");

        fclose(f_questions);

        fclose(f_reponses);

        return 1;

    }



    lanceBoucleEvenements();



    fclose(f_questions);

    fclose(f_reponses);

    libereDonneesImageRGB(&akinatorImage);

    return 0;

}



Noeud* creerNoeud(char *question) {

    Noeud *nouveau = (Noeud *)malloc(sizeof(Noeud));

    strcpy(nouveau->question, question);

    nouveau->oui = NULL;

    nouveau->non = NULL;

    return nouveau;

}



void construireArbre(Noeud *racine, FILE *f_questions) {

    char line[256];



    while (fgets(line, sizeof(line), f_questions) != NULL) {

        char *pos = strchr(line, ':');

        if (pos != NULL) {

            *pos = '\0';

            char *chemin = pos + 1;

            chemin[strcspn(chemin, "\n")] = '\0';  // Remove newline character

            Noeud *noeud = racine;

            while (*chemin) {

                if (*chemin == '1') {

                    if (noeud->oui == NULL) {

                        noeud->oui = creerNoeud(line);

                    }

                    noeud = noeud->oui;

                } else if (*chemin == '0') {

                    if (noeud->non == NULL) {

                        noeud->non = creerNoeud(line);

                    }

                    noeud = noeud->non;

                }

                chemin++;

            }

        }

    }

}



void obtenirQuestionSuivante(Noeud *racine, char *chemin, char *questionTexte) {

    Noeud *noeud = racine;

    while (*chemin) {

        if (*chemin == '1') {

            noeud = noeud->oui;

        } else if (*chemin == '0') {

            noeud = noeud->non;

        }

        chemin++;

    }

    strcpy(questionTexte, noeud->question);

}



void sauvegarderPays(const char *pays) {

    FILE *f = fopen("pays.txt", "r+");

    if (f == NULL) {

        perror("Erreur d'ouverture du fichier pays.txt");

        return;

    }



    // Vérifier si le pays est déjà dans le fichier

    char ligne[256];

    bool paysDejaPresent = false;

    while (fgets(ligne, sizeof(ligne), f) != NULL) {

        if (strncmp(ligne, pays, strlen(pays)) == 0) {

            paysDejaPresent = true;

            break;

        }

    }



    // Si le pays n'est pas déjà présent, l'ajouter à la fin du fichier

    if (!paysDejaPresent) {

        fseek(f, 0, SEEK_END); // Se positionner à la fin du fichier

        fprintf(f, "information: %s\n", pays);

    }



    fclose(f);

}



void sauvegarderQuestion(const char *chemin, const char *question) {

    FILE *f = fopen("questions.txt", "a+");

    if (f == NULL) {

        perror("Erreur d'ouverture du fichier questions.txt");

        return;

    }



    // Ajouter la question à la fin du fichier avec son chemin

    fprintf(f, "%s:%s\n", question, chemin);



    fclose(f);

}



void gestionEvenement(EvenementGfx evenement) {

    static bool pleinEcran = false;

    static char buffer[256] = "";

    static char paysSaisie[256] = "";

    static char questionSaisie[256] = "";



    switch (evenement) {

        case Initialisation:

            demandeTemporisation(20);

            break;



        case Temporisation:

            rafraichisFenetre();

            break;



        case Affichage:

            effaceFenetre(255, 255, 255);



            // Afficher l'image de fond

            if (akinatorImage != NULL) {

                ecrisImage(0, 0, LargeurFenetre, HauteurFenetre, akinatorImage->donneesRGB);

            }



            couleurCourante(0, 0, 0);

            epaisseurDeTrait(2);



            if (etat == QUESTION) {

                // Afficher la question dans un rectangle blanc avec bordure noire

                couleurCourante(255, 255, 255); // Blanc

                rectangle(90, 580, 890, 630);

                couleurCourante(0, 0, 0); // Noir

                afficheChaine(questionTexte, 24, 100, 600);



                // Afficher les boutons "Oui" et "Non" en rectangles jaunes

                couleurCourante(255, 255, 0); // Jaune

                rectangle(100, 180, 150, 220);

                rectangle(200, 180, 250, 220);

                couleurCourante(0, 0, 0); // Noir

                afficheChaine("Oui", 20, 110, 190);

                afficheChaine("Non", 20, 210, 190);



            } else if (etat == REJOUER) {

                // Afficher la question de rejouer dans un rectangle blanc avec bordure noire

                couleurCourante(255, 255, 255); // Blanc

                rectangle(90, 580, 890, 630);

                couleurCourante(0, 0, 0); // Noir

                afficheChaine("Voulez-vous rejouer ?", 24, 100, 600);



                // Afficher les boutons "Oui" et "Non" en rectangles jaunes

                couleurCourante(255, 255, 0); // Jaune

                rectangle(100, 220, 150, 260);

                rectangle(200, 220, 250, 260);

                couleurCourante(0, 0, 0); // Noir

                afficheChaine("Oui", 20, 110, 230);

                afficheChaine("Non", 20, 210, 230);



            } else if (etat == SAISIE_PAYS) {

                // Afficher la question de saisie de pays dans un rectangle blanc avec bordure noire

                couleurCourante(255, 255, 255); // Blanc

                rectangle(90, 580, 890, 630);

                couleurCourante(0, 0, 0); // Noir

                afficheChaine("Entrez le pays auquel vous pensiez :", 24, 100, 600);

                afficheChaine(buffer, 24, 100, 560);



                // Afficher le bouton "Valider" en rectangle jaune

                couleurCourante(255, 255, 0); // Jaune

                rectangle(100, 180, 250, 220);

                couleurCourante(0,0,0);

                afficheChaine("Valider",20,110,190);

         } else if (etat == SAISIE_QUESTION) {

            // Afficher la question de saisie de la question utilisateur dans un rectangle blanc avec bordure noire

            couleurCourante(255, 255, 255); // Blanc

            rectangle(90, 580, 890, 630);

            couleurCourante(0, 0, 0); // Noir

            afficheChaine("Entrez une question relative à ce pays :", 24, 100, 600);

            afficheChaine(buffer, 24, 100, 560);



            // Afficher le bouton "Valider" en rectangle jaune

            couleurCourante(255, 255, 0); // Jaune

            rectangle(100, 180, 250, 220);

            couleurCourante(0, 0, 0); // Noir

            afficheChaine("Valider", 20, 110, 190);



        } else if (etat == FIN_JEU) {

            // Afficher un message de fin

            couleurCourante(255, 255, 255); // Blanc

            rectangle(90, 580, 890, 630);

            couleurCourante(0, 0, 0); // Noir

            afficheChaine("Merci d'avoir joue !", 24, 100, 600);

        }



        break;



    case Clavier:

        if (etat == SAISIE_PAYS || etat == SAISIE_QUESTION) {

            if (caractereClavier() == 13) { // Entrée

                if (etat == SAISIE_PAYS) {

                    strcpy(paysSaisie, buffer);

                    sauvegarderPays(paysSaisie);

                    etat = SAISIE_QUESTION;

                } else if (etat == SAISIE_QUESTION) {

                    strcpy(questionSaisie, buffer);

                    sauvegarderQuestion(chemin, questionSaisie);

                    etat = REJOUER;

                }

                buffer[0] = '\0';

            } else if (caractereClavier() == 8) { 

                if (strlen(buffer) > 0) {

                    buffer[strlen(buffer) - 1] = '\0';

                }

            } else {

                char c = caractereClavier();

                strncat(buffer, &c, 1);

            }



            rafraichisFenetre();

        }

        break;



    case ClavierSpecial:

        break;



    case BoutonSouris:

        if (etat == QUESTION && etatBoutonSouris() == GaucheAppuye) {

            if (abscisseSouris() > 100 && abscisseSouris() < 150 && ordonneeSouris() > 180 && ordonneeSouris() < 220) {

                strcat(chemin, "1");

                questionCount++;

                if (questionCount < 7) {

                    obtenirQuestionSuivante(racine, chemin, questionTexte);

                } else {

                    etat = REJOUER;

                }

            } else if (abscisseSouris() > 200 && abscisseSouris() < 250 && ordonneeSouris() > 180 && ordonneeSouris() < 220) {

                strcat(chemin, "0");

                questionCount++;

                if (questionCount < 7) {

                    obtenirQuestionSuivante(racine, chemin, questionTexte);

                } else {

                    etat = SAISIE_PAYS;

                }

            }

            sauvegarderPays(questionTexte);

            rafraichisFenetre();

        } else if (etat == REJOUER && etatBoutonSouris() == GaucheAppuye) {

            if (abscisseSouris() > 100 && abscisseSouris() < 150 && ordonneeSouris() > 220 && ordonneeSouris() < 260) {

                // Rejouer

                strcpy(chemin, "");

                questionCount = 0;

                obtenirQuestionSuivante(racine, chemin, questionTexte);

                etat = QUESTION;

            } else if (abscisseSouris() > 200 && abscisseSouris() < 250 && ordonneeSouris() > 220 && ordonneeSouris() < 260) {

                // Quitter

                etat = FIN_JEU;

            }

            rafraichisFenetre();

        } else if (etat == SAISIE_PAYS && etatBoutonSouris() == GaucheAppuye) {

            if (abscisseSouris() > 100 && abscisseSouris() < 250 && ordonneeSouris() > 180 && ordonneeSouris() < 220) {

                // Valider le pays saisi

                strcpy(paysSaisie, buffer);

                sauvegarderPays(paysSaisie);

                buffer[0] = '\0'; // Reset buffer for the next input

                etat = SAISIE_QUESTION;

            }

            rafraichisFenetre();

        } else if (etat == SAISIE_QUESTION && etatBoutonSouris() == GaucheAppuye) {

            if (abscisseSouris() > 100 && abscisseSouris() < 250 && ordonneeSouris() > 180 && ordonneeSouris() < 220) {

                // Valider la question saisie

                strcpy(questionSaisie, buffer);

                sauvegarderQuestion(chemin, questionSaisie);

                buffer[0] = '\0'; // Reset buffer for the next input

                etat = REJOUER;

            }

            rafraichisFenetre();

        }

        break;



    case Inactivite:

        break;



    case Redimensionnement:

        break;

}

}