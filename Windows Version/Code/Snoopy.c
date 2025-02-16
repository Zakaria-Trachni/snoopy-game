#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

typedef struct {
    int up_left;
    int up_right;
    int down_left;
    int down_right;
} BallDirection;

typedef struct Ball {
    int x;
    int y;
    struct Ball *next;
} Ball;

typedef struct Player {
    int x;
    int y;
    struct Player *next;
} Player;

Player* playerHistory = NULL;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int x;
    int y;
} Bird;

typedef struct {
    int x;
    int y;
} Heart;

typedef struct RiskPosition {
    int x;
    int y;
    struct RiskPosition* next;
} RiskPosition;

typedef struct {
    RiskPosition* front;
    RiskPosition* rear;
    int count;
    int lastUpdateTime;
} RiskQueue;

typedef struct {
    char name[50];
    int score;
} PlayerScore;

typedef struct ScoreNode {
    PlayerScore data;
    struct ScoreNode* left;
    struct ScoreNode* right;
} ScoreNode;

ScoreNode* createScoreNode(PlayerScore playerScore) {
    ScoreNode* newNode = (ScoreNode*)malloc(sizeof(ScoreNode));
    newNode->data = playerScore;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

ScoreNode* insertPlayerScore(ScoreNode* root, PlayerScore playerScore) {
    if (root == NULL) {
        return createScoreNode(playerScore); 
    }

    ScoreNode* current = root;
    while (current != NULL) {
        if (strcmp(current->data.name, playerScore.name) == 0) {
            if (playerScore.score > current->data.score) {
                current->data.score = playerScore.score;
            }
            return root;
        } else if (strcmp(playerScore.name, current->data.name) < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (playerScore.score > root->data.score) {
        root->right = insertPlayerScore(root->right, playerScore);
    } else {
        root->left = insertPlayerScore(root->left, playerScore);
    }

    return root;
}

ScoreNode* loadScoresFromFile(const char* filename) {
    ScoreNode* root = NULL;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    PlayerScore playerScore;
    while (fscanf(file, "%s %d", playerScore.name, &playerScore.score) == 2) {
        root = insertPlayerScore(root, playerScore);
    }

    fclose(file);
    return root;
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int generateRandomSeed() {
    long number = time(NULL);
    int units = 0;
    int tens = 0;

    while (number % 10 != 0) {
        number--;
        units++;
    }

    while (number % 100 != 0) {
        number -= 10;
        tens += 10;
    }

    return units + tens;
}

int randomBetween(int min, int max) {
    int seed = generateRandomSeed();
    int randomNumber = (rand() + seed * 10) % (max - min + 1);

    if (randomNumber < min) {
        randomNumber = ((randomNumber + min) % 2 == 0) ? randomNumber + min + 1 : randomNumber + min;
        return randomNumber;
    } else {
        randomNumber = (randomNumber % 2 == 0) ? randomNumber + 1 : randomNumber;
        randomNumber = (randomNumber > max) ? randomNumber - 2 : randomNumber;
        return randomNumber;
    }
}

// Fonction auxiliaire pour sauvegarder les scores dans un fichier
void _saveScoresToFile(ScoreNode* node, FILE* f) {
    if (node != NULL) {
        _saveScoresToFile(node->left, f);
        fprintf(f, "%s %d\n", node->data.name, node->data.score);
        _saveScoresToFile(node->right, f);
    }
}

// Fonction pour sauvegarder les scores dans un fichier
void saveScoresToFile(ScoreNode* root, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return;
    }

    _saveScoresToFile(root, file);

    fclose(file);
}

void pushPlayerPosition(Player** top, int x, int y) {
    Player* newPosition = (Player*)malloc(sizeof(Player));
    newPosition->x = x;
    newPosition->y = y;
    newPosition->next = *top;
    *top = newPosition;
}

Position popPlayerPosition(Player** top) {
    if (*top == NULL) {
        Position invalidPosition = {-1, -1};
        return invalidPosition;
    }
    Position pos = {(*top)->x, (*top)->y};
    Player* temp = *top;
    *top = (*top)->next;
    free(temp);
    return pos;
}

int isPlayerHistoryEmpty(Player* top) {
    return top == NULL;
}

RiskQueue* createRiskQueue() {
    RiskQueue* queue = (RiskQueue*)malloc(sizeof(RiskQueue));
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
    queue->lastUpdateTime = 0;
    return queue;
}

void enqueueRisk(RiskQueue* queue, int x, int y) {
    RiskPosition* newRisk = (RiskPosition*)malloc(sizeof(RiskPosition));
    newRisk->x = x;
    newRisk->y = y;
    newRisk->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newRisk;
        queue->rear = newRisk;
    } else {
        queue->rear->next = newRisk;
        queue->rear = newRisk;
    }
    queue->count++;
}

void clearRisks(RiskQueue* queue) {
    RiskPosition* current = queue->front;
    while (current != NULL) {
        gotoxy(current->x, current->y);
        printf(" ");
        current = current->next;
    }
}

void updateRisks(RiskQueue* queue, int currentTime, int level, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5) {
    int riskFrequency = 50;

    if (level > 1) {
        riskFrequency = 50 / level;
    }

    if (currentTime - queue->lastUpdateTime >= riskFrequency) {
        clearRisks(queue);

        int numRisks = level;

        bool usedY[19] = {false};
        int i, j;

        usedY[bird1->y] = true;
        usedY[bird2->y] = true;
        usedY[bird3->y] = true;
        usedY[bird4->y] = true;
        usedY[bird5->y] = true;

        for (i = 0; i < numRisks; i++) {
            int y;
            int attempts = 0;

            do {
                y = randomBetween(5, 18);
                attempts++;

                if (attempts > 50) {
                    for (j = 5; j <= 18; j++) {
                        if (!usedY[j]) {
                            y = j;
                            break;
                        }
                    }
                    break;
                }

            } while (usedY[y]);

            usedY[y] = true;

            int x = randomBetween(1, 37);
            enqueueRisk(queue, x, y);
        }

        queue->lastUpdateTime = currentTime;
    }

    RiskPosition* current = queue->front;
    while (current != NULL) {
        gotoxy(current->x, current->y);
        printf("X");
        current = current->next;
    }
}

void freeRiskQueue(RiskQueue* queue) {
    while (queue->front != NULL) {
        RiskPosition* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
    free(queue);
}

int checkRiskCollision(RiskQueue* queue, Player* player) {
    RiskPosition* current = queue->front;
    while (current != NULL) {
        if (player->x == current->x && player->y == current->y) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void removeRisk(RiskQueue* queue, int x, int y) {
    RiskPosition* current = queue->front;
    RiskPosition* previous = NULL;

    while (current != NULL) {
        if (current->x == x && current->y == y) {
            if (previous == NULL) {
                queue->front = current->next;
            } else {
                previous->next = current->next;
            }
            if (current == queue->rear) {
                queue->rear = previous;
            }
            free(current);
            queue->count--;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void manageRisks(RiskQueue* queue, int currentTime, int level, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5) {
    updateRisks(queue, currentTime, level, bird1, bird2, bird3, bird4, bird5);
}

Player* createPlayer(int x, int y) {
    Player* player;
    player = (Player*)malloc(sizeof(Player));
    player->x = x;
    player->y = y;
    player->next = NULL;
    return player;
}

Bird* createBird(int x, int y) {
    Bird* bird = (Bird*)malloc(sizeof(Bird));
    bird->x = x;
    bird->y = y;
    return bird;
}

Ball* createBall(int x, int y) {
    Ball* ball;
    ball = (Ball*)malloc(sizeof(Ball));
    ball->x = x;
    ball->y = y;
    ball->next = NULL;
    return ball;
}

BallDirection* createBallDirections() {
    BallDirection *ball;
    ball = (BallDirection*)malloc(sizeof(BallDirection));
    ball->down_right = 1;
    ball->down_left = 0;
    ball->up_right = 0;
    ball->up_left = 0;
    return ball;
}

Heart* createHeart(int x, int y) {
    Heart* heart;
    heart = (Heart*)malloc(sizeof(Heart));
    heart->x = x;
    heart->y = y;
    return heart;
}

void setRedColor() {
    printf("\e[0;31m");
}

void resetColor() {
    printf("\e[0m");
}

void setPushablePositions(int xPositions[10], int yPositions[10]) {
    xPositions[0] = 2, xPositions[1] = 2, xPositions[2] = 18, xPositions[3] = 18;
    yPositions[0] = 5, yPositions[1] = 16, yPositions[2] = 5, yPositions[3] = 16;
}

void wait(long time) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = time;
    nanosleep(&ts, NULL);
}

void setConsoleColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void moveCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = FALSE;
    cursor.dwSize = 1;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}

void printRemainingLives(Heart *heart1, Heart *heart2, Heart *heart3) {
    moveCursor(1, 1);
    printf("Remaining Lives: ");
    setRedColor();
    printf("%c %c %c", 0x3, 0x3, 0x3);
    resetColor();
}

void printRemainingTime(int timer, int* timeLeft) {
    moveCursor(40, 1);
    printf("   ");
    moveCursor(40, 1);
    if (timer % 10 == 0 && timer != 0)
        printf("%d", (*timeLeft)--);
    else
        printf("%d", *timeLeft);
}

void printLevelNumber(int levelNumber) {
    moveCursor(1, 2);
    printf("\n\t\t Level: %d", levelNumber);
}

void setBallDirection(Ball *ball, BallDirection **directions) {
    if (ball->y == 5 && (*directions)->up_left == 1) {
        (*directions)->down_left = 1;
        (*directions)->up_left = 0;
    }
    if (ball->y == 5 && (*directions)->up_right == 1) {
        (*directions)->down_right = 1;
        (*directions)->up_right = 0;
    }
    if (ball->y == 18 && (*directions)->down_left == 1) {
        (*directions)->up_left = 1;
        (*directions)->down_left = 0;
    }
    if (ball->y == 18 && (*directions)->down_right == 1) {
        (*directions)->up_right = 1;
        (*directions)->down_right = 0;
    }
    if (ball->x == 1 && (*directions)->down_left == 1) {
        (*directions)->down_right = 1;
        (*directions)->down_left = 0;
    }
    if (ball->x == 1 && (*directions)->up_left == 1) {
        (*directions)->up_right = 1;
        (*directions)->up_left = 0;
    }
    if (ball->x == 37 && (*directions)->down_right == 1) {
        (*directions)->down_left = 1;
        (*directions)->down_right = 0;
    }
    if (ball->x == 37 && (*directions)->up_right == 1) {
        (*directions)->up_left = 1;
        (*directions)->up_right = 0;
    }
}

void setBallTrajectory(Ball** ball, BallDirection *directions) {
    if (directions->up_right == 1)
        (*ball)->next = createBall((*ball)->x + 2, (*ball)->y - 1);
    else if (directions->up_left == 1)
        (*ball)->next = createBall((*ball)->x - 2, (*ball)->y - 1);
    else if (directions->down_left == 1)
        (*ball)->next = createBall((*ball)->x - 2, (*ball)->y + 1);
    else if (directions->down_right == 1)
        (*ball)->next = createBall((*ball)->x + 2, (*ball)->y + 1);
}

void displayWelcomeScreen() {
    system("cls");
    setConsoleColor(14);
    int x = 14, y = 2;
    printf("\n");
    Sleep(50);
    moveCursor(x, y++);
    printf("  _________                                  \n");
    moveCursor(x, y++);
    Sleep(50);
    printf(" /   _____/ ____   ____   ____ ______ ___.__.\n");
    moveCursor(x, y++);
    Sleep(50);
    printf(" \\_____  \\ /    \\ /  _ \\ /  _ \\____ <   |  |\n");
    moveCursor(x, y++);
    Sleep(50);
    printf(" /        \\   |  (  <_> |  <_> )  |_> >___  |\n");
    moveCursor(x, y++);
    Sleep(50);
    printf("/_______  /___|  /\\____/ \\____/|   __// ____|\n");
    moveCursor(x, y++);
    Sleep(50);
    printf("        \\/     \\/              |__|   \\/     \n");
    moveCursor(x, y++);
    Sleep(50);
    printf("\n");
    moveCursor(x, y++);
    Sleep(50);
    printf("  Welcome to the Snoopy Adventure!\n");
    moveCursor(x, y++);
    Sleep(50);
    printf("  Press any key to start the journey...\n");
    moveCursor(x, y++);
    Sleep(50);
    getch();
    system("cls");
    return;
}

void displayLoadingAnimation() {
    setConsoleColor(14);
    char loadingText[] = "\n\n\n\t\t>Press {ENTER} to continue<";
    int p ,x,y;
    for (p= 0; p <= 100; p++) {
        system("cls");
        printf("\n\n\n\t\tLoading Game, Please Wait:\n\n\t\t\t\t %d%%\n\n", p);
        fflush(stdout);
        wait(10000000);
    }

    printf("\n\t\t\tLoading COMPLETE!\n");

    for (x = 0; loadingText[x] != '\0'; x++) {
        printf("%c", loadingText[x]);
        for (y = 0; y <= 10000000; y++) {}
    }
    getch();
    system("cls");
    int i, l;
    char welcomeMessage[] = "\n\n\n\n\t\t Welcome to Snoopy!";

    for (i = 0; i < strlen(welcomeMessage); i++) {
        printf("%c", welcomeMessage[i]);
    }
    puts("\n\n\n\t Press a key to continue");
    getch();
    system("cls");
}

void displayControls() {
    int x = 10, y = 5;
    system("cls");
    printf("Controls\n");
    moveCursor(x++, y++);
    printf("Use the following arrow keys to direct the Snoopy to the bird: ");
    moveCursor(x, y++);
    printf("Right Arrow");
    moveCursor(x, y++);
    printf("Left Arrow");
    moveCursor(x, y++);
    printf("Top Arrow");
    moveCursor(x, y++);
    printf("Bottom Arrow");
    moveCursor(x, y++);
    moveCursor(x, y++);
    printf("P & Esc pauses and escape the game.");
    moveCursor(x, y++);
    printf("Press any key to continue...");
    getch();
    return;
}

void printNumberOne() {
    printf("\n");
    moveCursor(25, 7);
    printf("     ,---, \n");
    moveCursor(25, 8);
    printf("  ,`--.' | \n");
    moveCursor(25, 9);
    printf(" /    /  : \n");
    moveCursor(25, 10);
    printf(":    |.' ' \n");
    moveCursor(25, 11);
    printf("`----':  | \n");
    moveCursor(25, 12);
    printf("   '   ' ; \n");
    moveCursor(25, 13);
    printf("   |   | | \n");
    moveCursor(25, 14);
    printf("   '   : ; \n");
    moveCursor(25, 15);
    printf("   |   | ' \n");
    moveCursor(25, 16);
    printf("   '   : | \n");
    moveCursor(25, 17);
    printf("   ;   |.' \n");
    moveCursor(25, 18);
    printf("   '---'   \n");
    moveCursor(25, 19);
    printf("\n");
}

void printNumberTwo() {
    printf("\n");
    int x = 25, y = 7;
    moveCursor(x, y++);
    printf("      ,----,\n");
    moveCursor(x, y++);
    printf("    .'   .' \\ \n");
    moveCursor(x, y++);
    printf("  ,----,'    |\n");
    moveCursor(x, y++);
    printf("  |    :  .  ;\n");
    moveCursor(x, y++);
    printf("  ;    |.'  /\n");
    moveCursor(x, y++);
    printf("  `----'/  ; \n");
    moveCursor(x, y++);
    printf("    /  ;  /  \n");
    moveCursor(x, y++);
    printf("   ;  /  /-, \n");
    moveCursor(x, y++);
    printf("  /  /  /.`|\n");
    moveCursor(x, y++);
    printf("./__;      :\n");
    moveCursor(x, y++);
    printf("|   :    .'\n");
    moveCursor(x, y++);
    printf(";   | .'\n");
    moveCursor(x, y++);
    printf("`---'\n");
}

void printNumberThree() {
    printf("\n");
    int x = 25, y = 7;
    moveCursor(x, y++);
    printf("  .--,-``-.\n");
    moveCursor(x, y++);
    printf(" /   /     '.\n");
    moveCursor(x, y++);
    printf("/ ../        ;\n");
    moveCursor(x, y++);
    printf("\\ ``\\  .`-    '\n");
    moveCursor(x, y++);
    printf(" \\___\\/   \\   :\n");
    moveCursor(x, y++);
    printf("      \\   :   |\n");
    moveCursor(x, y++);
    printf("      /  /   /\n");
    moveCursor(x, y++);
    printf("      \\  \\   \\\n");
    moveCursor(x, y++);
    printf("  ___ /   :   |\n");
    moveCursor(x, y++);
    printf(" /   /\\   /   :\n");
    moveCursor(x, y++);
    printf("/ ,,/  ',-    .\n");
    moveCursor(x, y++);
    printf("\\ ''\\        ;\n");
    moveCursor(x, y++);
    printf(" \\   \\     .'\n");
    moveCursor(x, y++);
    printf("  `--`-,,-'\n");
}

void displayWinScreen() {
    system("cls");
    setConsoleColor(10);
    int x = 3, y = 13;
    moveCursor(x, y++);
    printf("'##:::'##::'#######::'##::::'##::::'##:::::'##:'####:'##::: ##:'####:");
    Sleep(50);
    moveCursor(x, y++);
    printf(". ##:'##::'##.... ##: ##:::: ##:::: ##:'##: ##:. ##:: ###:: ##: ####:");
    Sleep(50);
    moveCursor(x, y++);
    printf(":. ####::: ##:::: ##: ##:::: ##:::: ##: ##: ##:: ##:: ####: ##: ####:");
    Sleep(50);
    moveCursor(x, y++);
    printf("::. ##:::: ##:::: ##: ##:::: ##:::: ##: ##: ##:: ##:: ## ## ##:: ##::");
    Sleep(50);
    moveCursor(x, y++);
    printf("::: ##:::: ##:::: ##: ##:::: ##:::: ##: ##: ##:: ##:: ##. ####::..:::");
    Sleep(50);
    moveCursor(x, y++);
    printf("::: ##:::: ##:::: ##: ##:::: ##:::: ##: ##: ##:: ##:: ##:. ###:'####:");
    Sleep(50);
    moveCursor(x, y++);
    printf("::: ##::::. #######::. #######:::::. ###. ###::'####: ##::. ##: ####:");
    Sleep(50);
    moveCursor(x, y++);
    printf(":::..::::::.......::::.......:::::::...::...:::....::..::::..::....::");
    moveCursor(x, y++);
    moveCursor(x, y++);
    moveCursor(x, y++);
    moveCursor(x, y++);
    getch();
    return;
}

void displayGameOverScreen() {
    system("cls");
    int x = 17, y = 3;
    setConsoleColor(12);
    moveCursor(x, y++);
    printf(":'######::::::'###::::'##::::'##:'########:\n");
    moveCursor(x, y++);
    printf("'##... ##::::'## ##::: ###::'###: ##.....::\n");
    moveCursor(x, y++);
    printf(" ##:::..::::'##:. ##:: ####'####: ##:::::::\n");
    moveCursor(x, y++);
    printf(" ##::'####:'##:::. ##: ## ### ##: ######:::\n");
    moveCursor(x, y++);
    printf(" ##::: ##:: #########: ##. #: ##: ##...::::\n");
    moveCursor(x, y++);
    printf(" ##::: ##:: ##.... ##: ##:.:: ##: ##:::::::\n");
    moveCursor(x, y++);
    printf(". ######::: ##:::: ##: ##:::: ##: ########:\n");
    moveCursor(x, y++);
    printf(":......::::..:::::..::..:::::..::........::\n");
    moveCursor(x, y++);
    printf(":'#######::'##::::'##:'########:'########::'####:\n");
    moveCursor(x, y++);
    printf("'##.... ##: ##:::: ##: ##.....:: ##.... ##: ####:\n");
    moveCursor(x, y++);
    printf(" ##:::: ##: ##:::: ##: ##::::::: ##:::: ##: ####:\n");
    moveCursor(x, y++);
    printf(" ##:::: ##: ##:::: ##: ######::: ########::: ##::\n");
    moveCursor(x, y++);
    printf(" ##:::: ##:. ##:: ##:: ##...:::: ##.. ##::::..:::\n");
    moveCursor(x, y++);
    printf(" ##:::: ##::. ## ##::: ##::::::: ##::. ##::'####:\n");
    moveCursor(x, y++);
    printf(". #######::::. ###:::: ########: ##:::. ##: ####:\n");
    moveCursor(x, y++);
    printf(":.......::::::...:::::........::..:::::..::....::\n");
    getch();
    system("cls");
    return;
}

void displayMainMenuAnimation() {
    printf(" ___  ___  ___  _____ _   _   ___  ___ _____ _   _ _   _ \n");
    Sleep(50);
    printf(" |  \\/  | / _ \\|_   _| \\ | |  |  \\/  ||  ___| \\ | | | | |\n");
    Sleep(50);
    printf(" | .  . |/ /_\\ \\ | | |  \\| |  | .  . || |__ |  \\| | | | |\n");
    Sleep(50);
    printf(" | |\\/| ||  _  | | | | . ` |  | |\\/| ||  __|| . ` | | | |\n");
    Sleep(50);
    printf(" | |  | || | | |_| |_| |\\  |  | |  | || |___| |\\  | |_| |\n");
    Sleep(50);
    printf(" \\_|  |_/\\_| |_/\\___/\\_| \\_/  \\_|  |_/\\____/\\_| \\_/\\___/ \n");
    Sleep(50);
}

char displayMainMenu(int level) {
    char option1[] = "-New Game\n";
    char option2[] = "-Continue\n";
    char option3[] = "-High Scores\n";
    char option4[] = "-Help\n";
    char option5[] = "-Exit\n";
    int selected;

    system("cls");

    displayMainMenuAnimation();

    printf("\n\n\n\n\n");
    Sleep(50);

    int k = 1;

    int x, y;
    int a = 23, b = 16;

    moveCursor(a, b++);
    printf("%d", k++);
    for (x = 0; option1[x] != '\0'; x++) {
        printf("%c", option1[x]);
        Sleep(50);
    }

    if (level != 1) {
        moveCursor(a, b++);
        printf("%d", k++);
        for (x = 0; option2[x] != '\0'; x++) {
            printf("%c", option2[x]);
            Sleep(50);
        }
    }

    moveCursor(a, b++);
    printf("%d", k++);
    for (x = 0; option3[x] != '\0'; x++) {
        printf("%c", option3[x]);
        Sleep(50);
    }

    moveCursor(a, b++);
    printf("%d", k++);
    for (x = 0; option4[x] != '\0'; x++) {
        printf("%c", option4[x]);
        Sleep(50);
    }

    moveCursor(a, b++);
    printf("%d", k++);
    for (x = 0; option5[x] != '\0'; x++) {
        printf("%c", option5[x]);
        Sleep(50);
    }

    selected = getch();
    return selected;
}

void displayHelp() {
    system("cls");
    setConsoleColor(14); // Couleur jaune pour le texte

    int x = 10, y = 5;

    moveCursor(x, y++);
    printf("=== Help ===\n");
    moveCursor(x, y++);
    printf("Welcome to Snoopy Adventure!\n");
    moveCursor(x, y++);
    printf("Here's a quick guide to help you understand the game:\n");
    y++;

    // Contr�les
    moveCursor(x, y++);
    printf("=== Controls ===\n");
    moveCursor(x, y++);
    printf("Use the arrow keys to move Snoopy:\n");
    moveCursor(x, y++);
    printf("-> Right Arrow: Move right\n");
    moveCursor(x, y++);
    printf("-> Left Arrow: Move left\n");
    moveCursor(x, y++);
    printf("-> Up Arrow: Move up\n");
    moveCursor(x, y++);
    printf("-> Down Arrow: Move down\n");
    moveCursor(x, y++);
    printf("-> 'B': Go back to the previous position\n");
    moveCursor(x, y++);
    printf("-> 'P': Pause the game\n");
    moveCursor(x, y++);
    printf("-> 'Esc': Exit the game\n");
    y++;

    // Symboles
    moveCursor(x, y++);
    printf("=== Symbols ===\n");
    moveCursor(x, y++);
    printf("Here's what each symbol represents:\n");
    moveCursor(x, y++);
    printf("-> %c: Snoopy (the player)\n", 0x1);
    moveCursor(x, y++);
    printf("-> %c: Ball (avoid it!)\n", 0xB);
    moveCursor(x, y++);
    printf("-> %c: Bird (catch it for points!)\n", 0xE);
    moveCursor(x, y++);
    printf("-> X: Risk (don't step on it!)\n");
    moveCursor(x, y++);
    printf("-> %c: Heart (your lives)\n", 0x3);
    y++;

    // Petit guide
    moveCursor(x, y++);
    printf("=== Gameplay Guide ===\n");
    moveCursor(x, y++);
    printf("1. Move Snoopy to catch birds (%c) and earn points.\n", 0xE);
    moveCursor(x, y++);
    printf("2. Avoid the ball (%c) and risks (X) to protect your lives.\n", 0xB);
    moveCursor(x, y++);
    printf("3. You have 3 lives (%c %c %c). Lose all lives, and it's game over!\n", 0x3, 0x3, 0x3);
    moveCursor(x, y++);
    printf("4. Complete each level by catching all birds before time runs out.\n");
    y++;

    moveCursor(x, y++);
    printf("Press any key to return to the main menu...");
    getch();
    system("cls");
}

void drawGameMap(Player *player, Ball *ball, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5, Heart *heart1, Heart *heart2, Heart *heart3) {
    system("cls");
    int i,j;
    for (i = 0; i <= 20; i++) {
        printf("\n");
        for (j = 0; j <= 44; j++) {
            if (i == 3 || i == 18) {
                if (j <= 39)
                    printf("%c", 0x16);
            } else {
                if (j == 0 || j == 39) {
                    if (i >= 3 && i <= 18)
                        printf("%c", 0xDF);
                } else if (i == player->y - 1 && j == player->x)
                    printf("%c", 0x1);
                else if (i == ball->y - 1 && j == ball->x)
                    printf("%c", 0xB);
                else if (i == bird1->y - 1 && j == bird1->x)
                    printf("%c", 0xE);
                else if (i == bird2->y - 1 && j == bird2->x)
                    printf("%c", 0xE);
                else if (i == bird3->y - 1 && j == bird3->x)
                    printf("%c", 0xE);
                else if (i == bird4->y - 1 && j == bird4->x)
                    printf("%c", 0xE);
                else if (i == bird5->y - 1 && j == bird5->x)
                    printf("%c", 0xE);
                else
                    printf(" ");
            }
        }
    }
}

void moveBall(Ball **ball, int ascii) {
    moveCursor((*ball)->x, (*ball)->y);
    printf("%c", ascii);
    Sleep(100);
    *ball = (*ball)->next;
    moveCursor((*ball)->x, (*ball)->y);
    printf("%c", 0xB);
}

void movePlayer(Player **player) {
    moveCursor((*player)->x, (*player)->y);
    printf("  ");
    Sleep(100);
    (*player) = (*player)->next;
    moveCursor((*player)->x, (*player)->y);
    printf("%c ", 0x1);
    moveCursor(25, 0);
    printf("%d, %d", (*player)->x, (*player)->y);
}

void getNextPlayerMove(Player** player) {
    int command;

    if (_kbhit()) {
        again:
        command = getch();

        if (command == 77 && (*player)->x < 37) {
            pushPlayerPosition(&playerHistory, (*player)->x, (*player)->y);
            (*player)->next = createPlayer((*player)->x + 2, (*player)->y);
            movePlayer(player);
        } else if (command == 75 && (*player)->x > 1) {
            pushPlayerPosition(&playerHistory, (*player)->x, (*player)->y);
            (*player)->next = createPlayer((*player)->x - 2, (*player)->y);
            movePlayer(player);
        } else if (command == 72 && (*player)->y > 5) {
            pushPlayerPosition(&playerHistory, (*player)->x, (*player)->y);
            (*player)->next = createPlayer((*player)->x, (*player)->y - 1);
            movePlayer(player);
        } else if (command == 80 && (*player)->y < 18) {
            pushPlayerPosition(&playerHistory, (*player)->x, (*player)->y);
            (*player)->next = createPlayer((*player)->x, (*player)->y + 1);
            movePlayer(player);
        } else if (command == 27) {
            exit(1);
        } else if (command == 'b' || command == 'B') {
            if (!isPlayerHistoryEmpty(playerHistory)) {
                Position pos = popPlayerPosition(&playerHistory);
                if (pos.x != -1 && pos.y != -1) {
                    (*player)->next = createPlayer(pos.x, pos.y);
                    movePlayer(player);
                }
            }
        } else if (command == 112) {
            printf("\n\nPress an arrow to continue.");
            while (!kbhit());
            goto again;
        }
    }
}

int checkBallCollision(Ball *ball, Player *player, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5) {
    if (ball->x == player->x && ball->y == player->y) {
        return 0;
    } else if (ball->x == bird1->x && ball->y == bird1->y) {
        return 1;
    } else if (ball->x == bird2->x && ball->y == bird2->y) {
        return 1;
    } else if (ball->x == bird3->x && ball->y == bird3->y) {
        return 1;
    } else if (ball->x == bird4->x && ball->y == bird4->y) {
        return 1;
    } else if (ball->x == bird5->x && ball->y == bird5->y) {
        return 1;
    }

    return -1;
}

void printCurrentScore(int score) {
    moveCursor(60, 1);
    printf("Score: %d", score);
}

void checkPlayerBirdCollision(Player *player, Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5, int number, int* totalScore) {
    if (player->y == bird1->y && player->x == bird1->x) {
        (*totalScore) += 20;
        printCurrentScore(*totalScore);
        bird1->y = 6 + number, bird1->x = 44;
        moveCursor(bird1->x, bird1->y);
        printf("%c", 0xE);
    } else if (player->y == bird2->y && player->x == bird2->x) {
        (*totalScore) += 20;
        printCurrentScore(*totalScore);
        bird2->y = 6 + number, bird2->x = 44;
        moveCursor(bird2->x, bird2->y);
        printf("%c", 0xE);
    } else if (player->y == bird3->y && player->x == bird3->x) {
        (*totalScore) += 20;
        printCurrentScore(*totalScore);
        bird3->y = 6 + number, bird3->x = 44;
        moveCursor(bird3->x, bird3->y);
        printf("%c", 0xE);
    } else if (player->y == bird4->y && player->x == bird4->x) {
        (*totalScore) += 20;
        printCurrentScore(*totalScore);
        bird4->y = 6 + number, bird4->x = 44;
        moveCursor(bird4->x, bird4->y);
        printf("%c", 0xE);
    } else if (player->y == bird5->y && player->x == bird5->x) {
        (*totalScore) += 20;
        printCurrentScore(*totalScore);
        bird5->y = 6 + number, bird5->x = 44;
        moveCursor(bird5->x, bird5->y);
        printf("%c", 0xE);
    }
}

void removeLife(int livesLeft, Heart *heart1, Heart *heart2, Heart *heart3) {
    if (livesLeft == 1) {
        moveCursor(heart3->x, heart3->y);
        printf(" ");
    } else if (livesLeft == 2) {
        moveCursor(heart2->x, heart2->y);
        printf(" ");
    } else if (livesLeft == 3) {
        moveCursor(heart1->x, heart1->y);
        printf(" ");
    }
}

void freePlayerHistory(Player **player) {
    Player *last;
    last = *player;
    *player = (*player)->next;

    while ((*player) != NULL) {
        free(last);
        last = *player;
        *player = (*player)->next;
    }

    free(last);
}

void freeBallHistory(Ball **ball, BallDirection *ballDirection) {
    Ball *last;
    last = *ball;
    *ball = (*ball)->next;

    while ((*ball) != NULL) {
        free(last);
        last = *ball;
        *ball = (*ball)->next;
    }

    free(last);
    free(ballDirection);
}

void freeHearts(Heart *heart1, Heart *heart2, Heart *heart3) {
    free(heart1);
    free(heart2);
    free(heart3);
}

void freeBirds(Bird *bird1, Bird *bird2, Bird *bird3, Bird *bird4, Bird *bird5) {
    free(bird1);
    free(bird2);
    free(bird3);
    free(bird4);
    free(bird5);
}

int totalScore = 0;

int playLevel(int levelNumber, ScoreNode** scoreTree, int* totalScore) {
    int gameOver = 0;
    int livesLeft = 3;
    Player* playerHistory = NULL;
    int timeLeft = 120 - (levelNumber * 10);
    int timer = 0, k = 0;

    RiskQueue* riskQueue = createRiskQueue();
    int riskTimer = 115 - (levelNumber * 5);

    Ball *ball = NULL, *ballHead = NULL;
    ball = createBall(randomBetween(1, 37), randomBetween(5, 19));
    ballHead = ball;

    BallDirection* ballDirections = NULL;
    ballDirections = createBallDirections();

    Player *player = NULL, *playerHead = NULL;
    player = createPlayer(randomBetween(1, 37), randomBetween(5, 19));
    playerHead = player;
    pushPlayerPosition(&playerHistory, player->x, player->y);

    Bird *bird1 = NULL, *bird2 = NULL, *bird3 = NULL, *bird4 = NULL, *bird5 = NULL;
    bird1 = createBird(randomBetween(1, 37), randomBetween(5, 19));
    bird2 = createBird(randomBetween(1, 37), randomBetween(5, 19));
    bird3 = createBird(randomBetween(1, 37), randomBetween(5, 19));
    bird4 = createBird(randomBetween(1, 37), randomBetween(5, 19));
    bird5 = createBird(randomBetween(1, 37), randomBetween(5, 19));

    Heart *heart1 = NULL, *heart2 = NULL, *heart3 = NULL;
    heart1 = createHeart(16, 1);
    heart2 = createHeart(18, 1);
    heart3 = createHeart(20, 1);

    drawGameMap(player, ball, bird1, bird2, bird3, bird4, bird5, heart1, heart2, heart3);
    printRemainingLives(heart1, heart2, heart3);
    printf("\tTime Left: ");
    printLevelNumber(levelNumber);
    printCurrentScore(*totalScore);

    int ballCollision = -1;
    int deadBirds = 0;

    do {
        wait(20000000 - (levelNumber * 1000000));

        manageRisks(riskQueue, timer, levelNumber, bird1, bird2, bird3, bird4, bird5);

        if (checkRiskCollision(riskQueue, player)) {
    livesLeft--;
    removeLife(livesLeft, heart1, heart2, heart3);
    removeRisk(riskQueue, player->x, player->y); // Supprime le risque apr�s collision
}

        setBallDirection(ball, &ballDirections);
        setBallTrajectory(&ball, ballDirections);

        if (ballCollision == -1)
            moveBall(&ball, 32);
        else if (ballCollision == 0) {
            moveBall(&ball, 0x1);
            removeLife(livesLeft, heart1, heart2, heart3);
        }
        else if (ballCollision == 1)
            moveBall(&ball, 0xE);

        ballCollision = checkBallCollision(ball, player, bird1, bird2, bird3, bird4, bird5);

        getNextPlayerMove(&player);

        checkPlayerBirdCollision(player, bird1, bird2, bird3, bird4, bird5, deadBirds, totalScore);

        if (player->x == ball->x && player->y == ball->y) {
            livesLeft--;
        }

        timer++;
        printRemainingTime(timer, &timeLeft);

        if ((bird1->x == 44 && bird2->x == 44 && bird3->x == 44 && bird4->x == 44 && bird5->x == 44) || livesLeft == 0 || timeLeft == 0) {
            gameOver = 1;
        }

    } while (!gameOver);

    freeRiskQueue(riskQueue);
    freeBallHistory(&ballHead, ballDirections);
    freePlayerHistory(&playerHead);
    freeHearts(heart1, heart2, heart3);

    while (!isPlayerHistoryEmpty(playerHistory)) {
        popPlayerPosition(&playerHistory);
    }

    if (bird1->x == 44 && bird2->x == 44 && bird3->x == 44 && bird4->x == 44 && bird5->x == 44) {
        freeBirds(bird1, bird2, bird3, bird4, bird5);
        return 1;
    } else {
        freeBirds(bird1, bird2, bird3, bird4, bird5);
        return 0;
    }
}


// Fonction auxiliaire pour afficher les scores
void _displayHighScores(ScoreNode* node) {
    if (node != NULL) {
        _displayHighScores(node->left);
        printf("%s: %d\n", node->data.name, node->data.score);
        _displayHighScores(node->right);
    }
}

// Fonction pour afficher les meilleurs scores
void displayHighScores(ScoreNode* root) {
    system("cls");

    if (root == NULL) {
        printf("Aucun score enregistr�.\n");
        printf("\nAppuyez sur une touche pour revenir au menu principal.\n");
        getch();
        return;
    }

    printf("\nMeilleurs scores :\n\n");

    _displayHighScores(root);

    getch();
}

int main() {
    ScoreNode* scoreTree = loadScoresFromFile("scores.txt");
    int level = 1;
    int selected, result;
    int totalScore = 0;

    hideCursor();
    displayLoadingAnimation();
    displayWelcomeScreen();

menu:
    selected = displayMainMenu(level);

    switch (selected) {
    case '1':
        system("cls");
        moveCursor(50, 7);
        Sleep(350);
        printNumberThree();
        Sleep(350);
        system("cls");

        printNumberTwo();
        Sleep(350);
        system("cls");

        printNumberOne();
        Sleep(300);

        level = 1;

        break;
    case '2':
        displayHighScores(scoreTree);
        goto menu; 
        break;
    case '3':
        if (level != 1) {
            goto menu;
        } else {
            displayHelp(); 
        }
        break;
    case '4':
        if (level != 1) {
            displayHelp(); 
            goto menu;
        } else {
            exit(1);
        }
        break;
    case '5':
        exit(1);
    default:
        break;
    }

    while (level < 6) {
        result = playLevel(level, &scoreTree, &totalScore); 

        if (result == 1) {
            displayWinScreen();
            level++;
        } else {
            displayGameOverScreen();

            char playerName[50];
            printf("    Enter your name: ");
            scanf("%s", playerName);

            PlayerScore playerScore;
            strcpy(playerScore.name, playerName);
            playerScore.score = totalScore;
            scoreTree = insertPlayerScore(scoreTree, playerScore);
            saveScoresToFile(scoreTree, "scores.txt");

            totalScore = 0; 
            goto menu;
        }
    }

    moveCursor(5, 10);
    printf("--- Total Score--- : %d", totalScore);

    char playerName[50];
    printf("     Enter your name: ");
    scanf("%s", playerName);

    PlayerScore playerScore;
    strcpy(playerScore.name, playerName);
    playerScore.score = totalScore;
    scoreTree = insertPlayerScore(scoreTree, playerScore);
    saveScoresToFile(scoreTree, "scores.txt");

    level = 1;
    totalScore = 0;

    goto menu;

    return 1;
}
