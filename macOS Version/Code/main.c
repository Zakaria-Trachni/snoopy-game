#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

// Board dimensions:
#define L 10
#define C 28
// Colors:
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"
// Clear the previous line:
#define CLEARLINE "\033[1A\033[K"

typedef struct {
    int x;
    int y;
} Coordonnee;

typedef struct {
    int up_left;
    int up_right;
    int down_left;
    int down_right;
} Ball_Directions;

typedef struct {
    Coordonnee *coordonnee;
    Ball_Directions *directions;
} Ball;

typedef struct player {
    int hearts;
    int birds;
    int score;
    int time;
    Coordonnee *coordonnee;
    struct player *next;
} Player;

typedef struct ElementPile {
    Coordonnee coordonnee;
    struct ElementPile *suivant;
} ElementPile;

typedef struct {
    ElementPile *sommet;
    int taille;
} Pile;

typedef struct ObstacleNode {
    char type;  // 'F' for Fire, 'C' for Crocodile, 'D' for Dog
    struct ObstacleNode* next;
} ObstacleNode;

typedef struct {
    ObstacleNode* front;
    ObstacleNode* rear;
    int size;
} ObstacleQueue;

typedef struct {
    Player *player;
    Ball *ball;
    char **siege;
    Pile mouvements;
    ObstacleQueue obstacles;
    int lastObstacleTime;
    char currentObstacleType;
    int obstaclesPlaced;  // Pour compter combien d'obstacles ont été placés
} Game;

// Prototypes des fonctions:
void initialiserPile(Pile *pile);
void empiler(Pile *pile, Coordonnee coord);
Coordonnee depiler(Pile *pile);
int estPileVide(Pile *pile);
void libererPile(Pile *pile);
void afficherHistoriqueMouvements(Pile *mouvements);
void PlayerHistory(Player* player);

// ------------------------- Pile -------------------------
void initialiserPile(Pile *pile) {
    pile->sommet = NULL;
    pile->taille = 0;
}

void empiler(Pile *pile, Coordonnee coord) {
    ElementPile *nouvelElement = (ElementPile*)malloc(sizeof(ElementPile));
    if (nouvelElement == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour l'élément de pile\n");
        return;
    }
    
    nouvelElement->coordonnee = coord;
    nouvelElement->suivant = pile->sommet;
    pile->sommet = nouvelElement;
    pile->taille++;
}

Coordonnee depiler(Pile *pile){
    Coordonnee coordVide = {-1, -1};
    
    if(estPileVide(pile))
        return coordVide;

    ElementPile *elementASupprimer = pile->sommet;
    Coordonnee coordDepilee = elementASupprimer->coordonnee;
    
    pile->sommet = elementASupprimer->suivant;
    free(elementASupprimer);
    pile->taille--;
    
    return coordDepilee;
}

int estPileVide(Pile *pile) {
    return pile->sommet == NULL;
}

void libererPile(Pile *pile) {
    while (!estPileVide(pile)) {
        depiler(pile);
    }
}

void afficherHistoriqueMouvements(Pile *mouvements){
    Pile piletmp;
    initialiserPile(&piletmp);
    Coordonnee coord = {4,6};
    empiler(&piletmp, coord);

    printf("\n==== Historique des Mouvements du Joueur ====\n");
    printf("Ordre : Du plus ancien au plus récent\n\n");

    int compteurMouvement = 0;
    while (!estPileVide(mouvements)) {
        Coordonnee coord = depiler(mouvements);
        empiler(&piletmp, coord);
    }
    depiler(&piletmp);

    while (!estPileVide(&piletmp)) {
        Coordonnee coord = depiler(&piletmp);
        printf("Mouvement %2d : (%2d, %2d)\n", ++compteurMouvement, coord.x, coord.y);
    }

    printf("\nNombre total de mouvements : %d\n", compteurMouvement);
    printf("============================================\n");
}

// ------------------------- File -------------------------
// Fonctions pour la file d'obstacles
void initQueue(ObstacleQueue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

void enqueue(ObstacleQueue* queue, char type) {
    ObstacleNode* newNode = (ObstacleNode*)malloc(sizeof(ObstacleNode));
    newNode->type = type;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

char dequeue(ObstacleQueue* queue) {
    if (queue->front == NULL) {
        return '\0';
    }

    ObstacleNode* temp = queue->front;
    char type = temp->type;
    queue->front = queue->front->next;
    free(temp);

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    queue->size--;
    return type;
}

// ------------------------- Other -------------------------
char fgetch() {
    char ch = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &ch, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return ch;
}

int mapChoice(){
    system("clear");
    printf(CYAN"\" Welcome to Snoopy game! \"\n\n"RESET);

    printf("###########################\n");
    printf("#                         #\n");
    printf("#                         #\n");
    printf("#       "GREEN"1_ Map 1"RESET"          #\n");
    printf("#       "RED"2_ Map 2"RESET"          #\n");
    printf("#                         #\n");
    printf("#                         #\n");
    printf("###########################\n");

    int choice = 0;
    do {
        printf("\nEnter your choice (1 or 2): ");
        scanf("%d", &choice);
        getchar();
    } while(choice != 1 && choice != 2);

    return choice;
}

void startMenu() {
    system("clear");
    printf(CYAN"\" Welcome to Snoopy game! \"\n\n"RESET);

    printf("###########################\n");
    printf("#                         #\n");
    printf("#                         #\n");
    printf("#        "GREEN"1_ PLAY"RESET"          #\n");
    printf("#        "RED"2_ EXIT"RESET"          #\n");
    printf("#                         #\n");
    printf("#                         #\n");
    printf("###########################\n");

    int choice;
    do {
        printf("\nEnter your choice (1 or 2): ");
        scanf("%d", &choice);
        getchar();
    } while(choice != 1 && choice != 2);

    if(choice == 2) {
        system("clear");
        printf("\n\t\tBest Score 🏆: ..");
        printf("\n\n           \" The game is " CYAN"finished! "RESET"\"\n");
        printf(CYAN"\n                 The end '^_^'\n\n"RESET);
        exit(0);
    } else if (choice == 1)
        system("clear");
}

void loading() {
    printf(GREEN"\n\n\t\t\tLoading...\n\n"RESET);
    for (int i = 0; i <= 55; i++) {
        for(int j = 0; j < 55; j++) {
            if(j < i) {
                printf(GREEN"🁢"RESET);
            } else
                printf("🁢");
        }
        printf("\n");
        fflush(stdout);
        usleep(5000);
        usleep(50000);
        if(i != 55) printf(CLEARLINE);
    }
    sleep(1);
}

void endLose(int birds, int score, Pile *mouvements, Player* player) {
    system("clear");
    printf(YELLOW "\n\tBirds 🐥: %d\tScore 🏅: %d" RESET, birds, score);
    printf("\n\n\t\tGame "RED"Over!"RESET);
    printf(RED"\n\n\t      The end '-_-'\n\n"RESET);
    
    afficherHistoriqueMouvements(mouvements);
    PlayerHistory(player);
    
    exit(0);
}

void endWin(int birds, int score, Pile *mouvements, Player* player) {
    system("clear");
    printf(YELLOW "\n\tBirds 🐥: %d\tScore 🏅: %d" RESET, birds, score);
    printf("\n\n\t\tYou "GREEN"Won!"RESET);
    printf(GREEN"\n\n\t  Congratulations '^_^'\n\n"RESET);
    
    afficherHistoriqueMouvements(mouvements);
    PlayerHistory(player);
    
    exit(0);
}

// ----------------------- Initialisation -----------------------
Ball* InitBall(){
    Ball* ball = (Ball*)malloc(sizeof(Ball));
    ball->coordonnee = (Coordonnee*)malloc(sizeof(Coordonnee));
    ball->directions = (Ball_Directions*)malloc(sizeof(Ball_Directions));
    
    ball->coordonnee->x = 1;
    ball->coordonnee->y = 22;
    
    ball->directions->down_right = 1;
    ball->directions->up_right = 0;
    ball->directions->down_left = 0;
    ball->directions->up_left = 0;
    
    return ball;
}

char **InitBoard(int level, char **siege) {
    if (siege == NULL) {
        siege = (char **)malloc(sizeof(char *) * L);
        for (int i = 0; i < L; i++)
            siege[i] = (char *)malloc(sizeof(char) * C);
    }

    if (level == 1){
        char *siegeLevel1[L] = {
            "###########################",
            "#B                       B#",
            "# #                     # #",
            "#            #            #",
            "#            #            #",
            "#            #            #",
            "#            #            #",
            "# #                     # #",
            "#B                       B#",
            "###########################"
        };
        for (int i = 0; i < L; i++) {
            strncpy(siege[i], siegeLevel1[i], C-1);
            siege[i][C-1] = '\0';
        }
    } else if (level == 2) {
        const char *siegeLevel2[L] = {
            "###########################",
            "#B       #               B#",
            "# #     # #             # #",
            "#      ##               # #",
            "#     ###   #           # #",
            "#           #           # #",
            "#           #           # #",
            "# #                     # #",
            "#B     ###              B #",
            "###########################"
        };
        for (int i = 0; i < L; i++) {
            strncpy(siege[i], siegeLevel2[i], C-1);
            siege[i][C-1] = '\0';
        }
    }
    return siege;
}

Game* InitGame(int level) {
    Game* game = (Game*)malloc(sizeof(Game));

    game->player = (Player*)malloc(sizeof(Player));
    game->player->coordonnee = (Coordonnee*)malloc(sizeof(Coordonnee));
    game->player->hearts = 3;
    game->player->birds = 0;
    game->player->score = 0;
    game->player->time = 120;
    game->player->coordonnee->x = 4;
    game->player->coordonnee->y = 6;
    game->player->next = NULL;
    
    initialiserPile(&game->mouvements);
    game->ball = InitBall();
    game->siege = InitBoard(level, NULL);
    
    initQueue(&game->obstacles);
    game->currentObstacleType = 'F';
    game->obstaclesPlaced = 0;
    game->lastObstacleTime = game->player->time;
    
    Coordonnee initialPos = {game->player->coordonnee->x, game->player->coordonnee->y};
    empiler(&game->mouvements, initialPos);
    
    game->siege[game->player->coordonnee->x][game->player->coordonnee->y] = 'P';
    game->siege[game->ball->coordonnee->x][game->ball->coordonnee->y] = 'O';
    
    return game;
}

Player* createPlayer(Player* currentPlayer) {
    Player* newPlayer = (Player*)malloc(sizeof(Player));
    if (newPlayer == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour Player\n");
        exit(1);
    }
    
    newPlayer->coordonnee = (Coordonnee*)malloc(sizeof(Coordonnee));
    if (newPlayer->coordonnee == NULL) {
        free(newPlayer);
        fprintf(stderr, "Erreur d'allocation mémoire pour Coordonnee\n");
        exit(1);
    }

    newPlayer->hearts = currentPlayer->hearts;
    newPlayer->birds = currentPlayer->birds;
    newPlayer->score = currentPlayer->score;
    newPlayer->time = currentPlayer->time;
    newPlayer->coordonnee->x = currentPlayer->coordonnee->x;
    newPlayer->coordonnee->y = currentPlayer->coordonnee->y;
    newPlayer->next = NULL;
    
    return newPlayer;
}

void appendPlayer(Game* game) {
    Player* newPlayer = createPlayer(game->player);

    newPlayer->next = game->player->next;
    game->player->next = newPlayer;
}

Player *reversePlayerList(Player *head) {
    Player *prev = NULL;
    Player *current = head;
    Player *next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

void PlayerHistory(Player* player) {
    printf("\n====== Historique des États du Joueur =======\n");
    printf("Position(x,y) | Vies | Oiseaux | Score | Temps\n\n");
    
    player = reversePlayerList(player);

    int moveCount = 1;
    Player* current = player;

    printf("État  1: ( 4, 6) | 3💝 | 0🐥 |  0🏅 | 120s\n");
    while (current != NULL) {
        moveCount++;
        printf("État %2d: (%2d,%2d) | %d💝 | %d🐥 | %2d🏅 | %3ds\n", moveCount, current->coordonnee->x, current->coordonnee->y, current->hearts, current->birds, current->score, current->time);
        current = current->next;
    }
    
    printf("\nNombre total d'états enregistrés: %d\n", moveCount);
    printf("============================================\n");
}

void freePlayerList(Player* player) {
    Player* current = player->next;
    while (current != NULL) {
        Player* next = current->next;
        free(current->coordonnee);
        free(current);
        current = next;
    }
}

char getNextObstacleType(char currentType) {
    if (currentType == 'F') return 'C';
    if (currentType == 'C') return 'D';
    return 'F';
}

void refreshScreen(Game *game) {
    system("clear");
    printf("");
    for(int i = 0; i < game->player->hearts; i++)
        printf("💝");
    printf("\t   🕑%3d   ", game->player->time);
    for(int i = 0; i < game->player->birds; i++)
        printf("🐥");
    printf("\n");
    
    for(int i = 0; i < L; i++) {
        for (int j = 0; j < C; j++) {
            if(game->siege[i][j] == 'B')
                printf(PURPLE "%c" RESET, game->siege[i][j]);
            else if (game->siege[i][j] == 'P')
                printf(GREEN "%c" RESET, game->siege[i][j]);
            else if (game->siege[i][j] == 'O')
                printf(RED "%c" RESET, game->siege[i][j]);
            else if(game->siege[i][j] == 'F')
                printf(RED "%c" RESET, game->siege[i][j]);
            else if(game->siege[i][j] == 'C')
                printf(CYAN "%c" RESET, game->siege[i][j]);
            else if(game->siege[i][j] == 'D')
                printf(YELLOW "%c" RESET, game->siege[i][j]);
            else
                printf("%c", game->siege[i][j]);
        }
        printf("\n");
    }
}

void placeObstacle(Game *game, char obstacleType) {
    int x, y;
    int attempts = 0;
    const int MAX_ATTEMPTS = 100;

    do {
        x = 1 + rand() % (L-2);
        y = 1 + rand() % (C-2);
        attempts++;
        if (attempts > MAX_ATTEMPTS) return;
    } while (game->siege[x][y] != ' ');
    
    game->siege[x][y] = obstacleType;
    game->obstaclesPlaced++;

    if (game->obstaclesPlaced >= 3) {
        game->currentObstacleType = getNextObstacleType(game->currentObstacleType);
        game->obstaclesPlaced = 0;
    }
}

// ----------------------- Ball Movement Functions -----------------------
void swapAndMove(Game *game, int newX, int newY) {
    char currentPos = game->siege[game->ball->coordonnee->x][game->ball->coordonnee->y];
    char nextPos = game->siege[newX][newY];

    if(currentPos != 'F' && currentPos != 'C' && currentPos != 'D')
        game->siege[game->ball->coordonnee->x][game->ball->coordonnee->y] = ' ';

    char tempObstacle = (nextPos == 'F' || nextPos == 'C' || nextPos == 'D') ? nextPos : ' ';
    
    game->siege[newX][newY] = 'O';
    game->ball->coordonnee->x = newX;
    game->ball->coordonnee->y = newY;
    
    if (tempObstacle != ' ')
        game->siege[newX][newY] = tempObstacle;
}

void setDirection(Ball *ball, int ru, int lu, int rd, int ld) {
    ball->directions->up_right = ru;
    ball->directions->up_left = lu;
    ball->directions->down_right = rd;
    ball->directions->down_left = ld;
}

void handleBounce(Game *game, int newX, int newY, int direction) {
    char nextPos = game->siege[newX][newY];
    if (nextPos != 'F' && nextPos != 'C' && nextPos != 'D')
        game->siege[game->ball->coordonnee->x][game->ball->coordonnee->y] = nextPos;
    else
        game->siege[game->ball->coordonnee->x][game->ball->coordonnee->y] = ' ';
    
    game->siege[newX][newY] = 'O';
    game->ball->coordonnee->x = newX;
    game->ball->coordonnee->y = newY;
    
    refreshScreen(game);
    usleep(100000);
    setDirection(game->ball, direction == 0, direction == 1, direction == 2, direction == 3);
}

int isPositionAvailable(char pos) {
    return pos == ' ' || pos == 'P' || pos == 'F' || pos == 'C' || pos == 'D';
}

int isTraversableByBall(char pos) {
    return (pos == ' ' || pos == 'F' || pos == 'C' || pos == 'D');
}

void *ballMove(void *arg) {
    Game *game = (Game*)arg;
    while(1) {
        // Gestion du temps et des obstacles
        if (game->lastObstacleTime - game->player->time >= 30) {
            for (int i = 0; i < 3 && game->obstaclesPlaced < 3; i++) {
                placeObstacle(game, game->currentObstacleType);
            }
            game->lastObstacleTime = game->player->time;
        }

        Ball *ball = game->ball;
        int x = ball->coordonnee->x;
        int y = ball->coordonnee->y;

        // Handle up-right movement
        if(ball->directions->up_right) {
            if(game->siege[x-1][y+1] == 'P') {
                game->player->hearts--;
                if(game->player->hearts == 0)
                    endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

                if(isTraversableByBall(game->siege[x-2][y+2])) {
                    swapAndMove(game, x-2, y+2);
                } else if(game->siege[x-2][y+2] == '#' && isTraversableByBall(game->siege[x][y+2])) {
                    handleBounce(game, x, y+2, 2);
                } else if(game->siege[x-2][y+2] == '#' && game->siege[x][y+2] == '#') {
                    handleBounce(game, x-1, y+1, 2);
                }
            } else if(isTraversableByBall(game->siege[x-1][y+1])) {
                swapAndMove(game, x-1, y+1);
                setDirection(ball, 1, 0, 0, 0);
            } else if(game->siege[x-1][y+1] == '#' && isTraversableByBall(game->siege[x-1][y-1])) {
                handleBounce(game, x-1, y-1, 1);
            } else if(game->siege[x-1][y+1] == '#' && game->siege[x-1][y-1] == '#') {
                handleBounce(game, x+1, y+1, 2);
            }
        }
        
        // Handle down-right movement (même logique)
        else if(ball->directions->down_right) {
            if(game->siege[x+1][y+1] == 'P') {
                game->player->hearts--;
                if(game->player->hearts == 0)
                    endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

                if(isPositionAvailable(game->siege[x+2][y+2])) {
                    swapAndMove(game, x+2, y+2);
                } else if(game->siege[x+2][y+2] == '#' && isPositionAvailable(game->siege[x+2][y])) {
                    handleBounce(game, x+2, y, 3);
                } else if(game->siege[x+2][y+2] == '#' && game->siege[x+2][y] == '#') {
                    handleBounce(game, x, y, 0);
                }
            } else if(isPositionAvailable(game->siege[x+1][y+1])) {
                swapAndMove(game, x+1, y+1);
                setDirection(ball, 0, 0, 1, 0);
            } else if(game->siege[x+1][y+1] == '#' && isPositionAvailable(game->siege[x+1][y-1])) {
                handleBounce(game, x+1, y-1, 3);
            } else if(game->siege[x+1][y+1] == '#' && game->siege[x+1][y-1] == '#') {
                handleBounce(game, x-1, y+1, 0);
            }
        }
        
        // Handle up-left movement (même logique)
        else if(ball->directions->up_left) {
            if(game->siege[x-1][y-1] == 'P') {
                game->player->hearts--;
                if(game->player->hearts == 0)
                    endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

                if(isPositionAvailable(game->siege[x-2][y-2])) {
                    swapAndMove(game, x-2, y-2);
                } else if(game->siege[x-2][y-2] == '#' && isPositionAvailable(game->siege[x][y-2])) {
                    handleBounce(game, x, y-2, 3);
                } else if(game->siege[x-2][y-2] == '#' && game->siege[x][y-2] == '#') {
                    handleBounce(game, x, y, 3);
                }
            } else if(isPositionAvailable(game->siege[x-1][y-1])) {
                swapAndMove(game, x-1, y-1);
                setDirection(ball, 0, 1, 0, 0);
            } else if(game->siege[x-1][y-1] == '#' && isPositionAvailable(game->siege[x-1][y+1])) {
                handleBounce(game, x-1, y+1, 0);
            } else if(game->siege[x-1][y-1] == '#' && game->siege[x-1][y+1] == '#') {
                handleBounce(game, x+1, y-1, 3);
            }
        }
        
        // Handle down-left movement (même logique)
        else if(ball->directions->down_left) {
            if(game->siege[x+1][y-1] == 'P') {
                game->player->hearts--;
                if(game->player->hearts == 0)
                    endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

                if(isPositionAvailable(game->siege[x+2][y-2])) {
                    swapAndMove(game, x+2, y-2);
                } else if(game->siege[x+2][y-2] == '#' && isPositionAvailable(game->siege[x][y-2])) {
                    handleBounce(game, x, y-2, 1);
                } else if(game->siege[x+2][y-2] == '#' && game->siege[x][y-2] == '#') {
                    handleBounce(game, x, y, 0);
                }
            } else if(isPositionAvailable(game->siege[x+1][y-1])) {
                swapAndMove(game, x+1, y-1);
                setDirection(ball, 0, 0, 0, 1);
            } else if(game->siege[x+1][y-1] == '#' && isPositionAvailable(game->siege[x+1][y+1])) {
                handleBounce(game, x+1, y+1, 2);
            } else if(game->siege[x+1][y-1] == '#' && game->siege[x+1][y+1] == '#') {
                handleBounce(game, x-1, y-1, 1);
            }
        }

        game->player->time--;
        if(game->player->time == 0)
            endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

        refreshScreen(game);
        usleep(100000);
    }
    return NULL;
}

// ----------------------- Player Movement Functions -----------------------
void *playerMove(void *arg) {
    Game *game = (Game*)arg;
    char ch;
    int x, y;
    
    while(1) {
        x = game->player->coordonnee->x;
        y = game->player->coordonnee->y;
        
        ch = fgetch();
        
        Coordonnee oldPos = {x, y};
        int newX = x;
        int newY = y;
        
        // Déterminer la nouvelle position potentielle
        if ((ch == 'i' || ch=='I') && game->siege[x-1][y] != '#') {
            newX = x-1;
        } else if ((ch == 'k' || ch=='K') && game->siege[x+1][y] != '#') {
            newX = x+1;
        } else if ((ch == 'j' || ch=='J') && game->siege[x][y-1] != '#') {
            newY = y-1;
        } else if ((ch == 'l' || ch=='L') && game->siege[x][y+1] != '#') {
            newY = y+1;
        } else if (ch == 'u' || ch == 'U') {
            if (!estPileVide(&game->mouvements)) {
                Coordonnee prevPos = depiler(&game->mouvements);
                if (prevPos.x != x || prevPos.y != y) {
                    game->siege[x][y] = ' ';
                    game->player->coordonnee->x = prevPos.x;
                    game->player->coordonnee->y = prevPos.y;
                }
            }
            continue;
        }

        char newPos = game->siege[newX][newY];
        if (newPos == 'F' || newPos == 'C' || newPos == 'D') {
            endLose(game->player->birds, game->player->score, &game->mouvements, game->player);
        }

        // Si un mouvement a été effectué
        if (newX != x || newY != y) {
            game->siege[x][y] = ' ';
            game->player->coordonnee->x = newX;
            game->player->coordonnee->y = newY;

            empiler(&game->mouvements, oldPos);
            appendPlayer(game);
        }

        if(game->siege[newX][newY] == 'B') {
            game->player->birds++;
            game->player->score = game->player->birds * 5;
            game->siege[newX][newY] = ' ';
        }

        if(game->player->hearts == 0)
            endLose(game->player->birds, game->player->score, &game->mouvements, game->player);

        if(game->player->birds == 4) {
            refreshScreen(game);
            usleep(1000000);
            endWin(game->player->birds, game->player->score, &game->mouvements, game->player);
        }

        game->siege[newX][newY] = 'P';
        refreshScreen(game);
    }
    return NULL;
}

// ----------------------- Clear Memory Function -----------------------
void clearAll(Game *game){
    if(game == NULL)
        return;

    while (game->obstacles.front != NULL)
        dequeue(&game->obstacles);

    freePlayerList(game->player);

    if(game->player != NULL){
        if(game->player->coordonnee != NULL)
            free(game->player->coordonnee);
        free(game->player);
    }

    if(game->ball != NULL){
        if(game->ball->coordonnee != NULL)
            free(game->ball->coordonnee);
        if(game->ball->directions != NULL)
            free(game->ball->directions);
        free(game->ball);
    }

    if(game->siege != NULL){
        for (int i = 0; i < L; i++) {
            if (game->siege[i] != NULL)
                free(game->siege[i]);
        }
        free(game->siege);
    }

    libererPile(&game->mouvements);
    free(game);
}

int main() {
    srand(time(NULL));
    pthread_t ballThread, playerThread;

    int map = mapChoice();
    Game *game = InitGame(map);

    startMenu();
    loading();
    
    refreshScreen(game);
    pthread_create(&ballThread, NULL, ballMove, game);
    pthread_create(&playerThread, NULL, playerMove, game);
    
    pthread_join(ballThread, NULL);
    pthread_join(playerThread, NULL);

    clearAll(game);
}