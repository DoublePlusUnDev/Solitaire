#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#define LEADERBOARD_NAME "solitaire-leaderboard.txt"
#define LEADERBOARD_LENGTH 10
#define MIN_NAME_LENGTH 4
#define MAX_NAME_LENGTH 12

typedef struct UserData
{
    struct UserData* prev;
    struct UserData* next;
    char name[MAX_NAME_LENGTH + 1];
    int games;
    int wins;
} UserData;

//retuns true upon successful login, returns false upon exiting the game
bool ReadUserName();

//reads the leaderboard file if it exists
void LoadLeaderboard();

//frees the leaderboard's memory
void FreeLeaderboard();

//method to call when a game is started
void RegisterGame();

//method to call when a game is won
void RegisterWin();

#endif