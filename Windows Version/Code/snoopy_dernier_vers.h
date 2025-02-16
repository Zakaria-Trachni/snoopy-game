#ifndef SNOOPY_DERNIER_VERS_H
#define SNOOPY_DERNIER_VERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

// Structure pour represeenter la direction de la balle
typedef struct {
    int up_left;    // Direction haut-gauche
    int up_right;   // Direction haut-droite
    int down_left;  // Direction bas-gauche
    int down_right; // Direction bas-droite
} BallDirection;

// Structure pour represeenter la balle
typedef struct Ball {
    int x;          // Position x de la balle
    int y;          // Position y de la balle
    struct Ball *next; // Pointeur vers la prochaine position de la balle
} Ball;

// Structure pour represeenter le joueur
typedef struct Player {
    int x;          // Position x du joueur
    int y;          // Position y du joueur
    struct Player *next; // Pointeur vers la prochaine position du joueur
} Player;

// Historique des positions du joueur
Player* playerHistory;

// Structure pour representer une position
typedef struct {
    int x;          // Position x
    int y;          // Position y
} Position;

// Structure pour representer un oiseau
typedef struct {
    int x;          // Position x de l'oiseau
    int y;          // Position y de l'oiseau
} Bird;

// Structure pour representer un coeur (vie)
typedef struct {
    int x;          // Position x du cœur
    int y;          // Position y du cœur
} Heart;

// Structure pour representer une position au� risque
typedef struct RiskPosition {
    int x;          // Position x du risque
    int y;          // Position y du risque
    struct RiskPosition* next; // Pointeur vers la prochaine position à risque
} RiskPosition;

// Structure pour representer une file de positions au� risque
typedef struct {
    RiskPosition* front; // Début de la file
    RiskPosition* rear;  // Fin de la file
    int count;           // Nombre de risques dans la file
    int lastUpdateTime;  // Dernière mise à jour des risques
} RiskQueue;

// Structure pour representer le score d'un joueur
typedef struct {
    char name[50];  // Nom du joueur
    int score;      // Score du joueur
} PlayerScore;

// Structure pour representer un noeud dans l'arbre des scores
typedef struct ScoreNode {
    PlayerScore data;       // Donnees du score
    struct ScoreNode* left; // Fils gauche
    struct ScoreNode* right;// Fils droit
} ScoreNode;

// Fonctions pour manipuler les scores
ScoreNode* createScoreNode(PlayerScore playerScore);
ScoreNode* insertPlayerScore(ScoreNode* root, PlayerScore playerScore);
ScoreNode* loadScoresFromFile(const char* filename);
void _saveScoresToFile(ScoreNode* node, FILE* f);
void saveScoresToFile(ScoreNode* root, const char* filename);

// Fonctions pour manipuler les positions du joueur
void pushPlayerPosition(Player** top, int x, int y);
Position popPlayerPosition(Player** top);
int isPlayerHistoryEmpty(Player* top);

// Fonctions pour manipuler les risques
RiskQueue* createRiskQueue();
void enqueueRisk(RiskQueue* queue, int x, int y);
void clearRisks(RiskQueue* queue);
void updateRisks(RiskQueue* queue, int currentTime, int level, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5);
void freeRiskQueue(RiskQueue* queue);
int checkRiskCollision(RiskQueue* queue, Player* player);
void removeRisk(RiskQueue* queue, int x, int y);
void manageRisks(RiskQueue* queue, int currentTime, int level, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5);

// Fonctions pour manipuler l'affichage
void gotoxy(int x, int y);
void setRedColor();
void resetColor();
void moveCursor(int x, int y);
void hideCursor();
void printRemainingLives(Heart *heart1, Heart *heart2, Heart *heart3);
void printRemainingTime(int timer, int* timeLeft);
void printLevelNumber(int levelNumber);
void printCurrentScore(int score);

// Fonctions pour manipuler la balle
void setBallDirection(Ball *ball, BallDirection **directions);
void setBallTrajectory(Ball** ball, BallDirection *directions);
int checkBallCollision(Ball *ball, Player *player, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5);

// Fonctions pour manipuler le joueur
void movePlayer(Player **player);
void getNextPlayerMove(Player** player);
void checkPlayerBirdCollision(Player *player, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5, int number, int* totalScore);
void removeLife(int livesLeft, Heart *heart1, Heart *heart2, Heart *heart3);

// Fonctions pour liberer la memoire
void freePlayerHistory(Player **player);
void freeBallHistory(Ball **ball, BallDirection *ballDirection);
void freeHearts(Heart *heart1, Heart *heart2, Heart *heart3);
void freeBirds(Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5);

// Fonctions pour l'affichage des ecrans
void displayWelcomeScreen();
void displayLoadingAnimation();
void displayControls();
void displayWinScreen();
void displayGameOverScreen();
void displayMainMenuAnimation();
char displayMainMenu(int level);
void displayHelp();
void drawGameMap(Player *player, Ball *ball, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5, Heart *heart1, Heart *heart2, Heart *heart3);

// Fonction principale du jeu
int playLevel(int levelNumber, ScoreNode** scoreTree, int* totalScore);

// Fonctions pour afficher les scores
void _displayHighScores(ScoreNode* node);
void displayHighScores(ScoreNode* root);

#endif // SNOOPY_DERNIER_VERS_H
