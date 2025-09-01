#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "logging.h"
#include "leaderboard.h"

#include "debugmalloc.h"

UserData* currentUser;
UserData* leaderboard;

//removes all occurences of character from a string
static void RemoveChar(char* str, char ch)
{
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] != ch)
        {
            str[j] = str[i];
            j++;
        }
    }

    str[j] = '\0';
}

//calculates a player's score based on wins and games
//score = 100 * win^2 / games
static int CalculateScore(UserData user)
{
    if (user.games == 0)
        return 0;
    else
        return ((100 * user.wins) * user.wins) / user.games;
}

//swap user with the one above it
static void SwapWithNext(UserData* user)
{
    if (user->next == NULL)
        return;

    UserData* nextUser = user->next;

    if (user->prev != NULL)
        user->prev->next = nextUser;
    if (nextUser->next != NULL)
        nextUser->next->prev = user;

    user->next = nextUser->next;
    nextUser->next = user;

    nextUser->prev = user->prev;
    user->prev = nextUser;
}

//swaps until the user one below has a lesser and the user one above has a higher score
static void AdjustUserOrder(UserData* user)
{
    //swap with previous(to higher rank)
    while (user->prev != NULL && CalculateScore(*user) > CalculateScore(*user->prev))
    {
        SwapWithNext(user->prev);
        if (user->prev == NULL)
            leaderboard = user;
    }

    //swap with next(to lower rank)
    while (user->next != NULL && CalculateScore(*user) < CalculateScore(*user->next))
    {
        SwapWithNext(user);
        if (user->prev->prev == NULL)
            leaderboard = user->prev;
    }
}

//appends a new user to the board and sorts in into the correct place
static UserData* AddNewUser(UserData* entry)
{
    entry->prev = NULL;
    entry->next = leaderboard;
    if (leaderboard != NULL)
        leaderboard->prev = entry;

    leaderboard = entry;
    
    AdjustUserOrder(entry);
    return entry;
}

//selects the user by the given name as the current user
//if it doesn't exist, creates one
static void RegisterUser(char userName[MAX_NAME_LENGTH + 1])
{
    LOG_ARGS("PLAYER LOGGED IN UNDER USERNAME: %s\n", userName);
    currentUser = leaderboard;
    while (currentUser != NULL)
    {
        if (strcmp(currentUser->name, userName) == 0)
        {
            LOG_ARGS("OLD USER REGISTERED AS %s\n", userName);
            break;
        }
        currentUser = currentUser->next;
    }
    
    if (currentUser == NULL)
    {
        currentUser = (UserData*)malloc(sizeof(UserData));
        strcpy(currentUser->name, userName);
        currentUser->wins = 0;
        currentUser->games = 0;
        LOG_ARGS("NEW USER REGISTERED AS %s\n", userName);
        currentUser = AddNewUser(currentUser);
    }
}

//prints the leaderboard sign and {LEADERBOARD_LENGTH} users below
static void DisplayLeaderboard()
{
    move(3, 0);
    printw("LEADERBOARD:");      
    move(4, MAX_NAME_LENGTH - 4);
    printw("Name    Score   Wins  Games");
    int i = 0;
    UserData* current = leaderboard;
    while (i < LEADERBOARD_LENGTH && current != NULL)
    {
        move(5 + i, MAX_NAME_LENGTH - strlen(current->name));
        printw("%s %8d %6d %6d", current->name, CalculateScore(*current), current->wins, current->games);

        current = current->next;
        i++;
    }
}

//reads in the username and registers the user
//also displays the leaderboard below
bool ReadUserName()
{
    char userName[MAX_NAME_LENGTH + 1];

    //fill up name buffer with zeros
    for (int i = 0; i <= MAX_NAME_LENGTH; i++)
        userName[i] = '\0';
    
    int nextLetter = 0;
    while (true)
    {
        clear();
        //sets to default terminal color
        attrset(COLOR_PAIR(0));
        move(0, 0);
        printw("Enter your username:");
        DisplayLeaderboard();
        move(1, 0);
        printw("%s", userName);
        refresh();
        int letter = getch();

        //enter pressed after a long enough name
        if (letter == '\n' && MIN_NAME_LENGTH <= nextLetter)
        {
            RegisterUser(userName);
            return true;
        }
        //escape to quit
        else if (letter == 27)
        {
            return false;
        }
        else if (nextLetter < MAX_NAME_LENGTH && (((int)'a' <= letter && letter <= (int)'z') || 
                                                  ((int)'A' <= letter && letter <= (int)'Z')  ||
                                                  ((int)'0' <= letter && letter <= (int)'9')))
        {
            userName[nextLetter] = (char)letter;
            nextLetter++;
        }
        else if(0 < nextLetter && letter == KEY_BACKSPACE)
        {
            nextLetter--;
            userName[nextLetter] = '\0';
        }
    }

}

//reads the leaderboard file
void LoadLeaderboard()
{
    FILE* dataFile = fopen(LEADERBOARD_NAME, "r");

    if (dataFile == NULL) 
        return;

    char buffer[100];
    leaderboard = NULL;
    currentUser = NULL;
    
    while (fgets(buffer, 100, dataFile) != NULL)
    {
        RemoveChar(buffer, ' ');
        char* name = strtok(buffer, ",");

        if (strlen(name) < MIN_NAME_LENGTH || strlen(name) > MAX_NAME_LENGTH)
            continue;

        char* winsStr = strtok(NULL, ",");
        int wins = atoi(winsStr);

        char* gamesStr = strtok(NULL, ",");
        int games = atoi(gamesStr);

        UserData* newUser = (UserData*) malloc(sizeof(UserData));
        strcpy(newUser->name, name);
        newUser->wins = wins;
        newUser->games = games;
        
        AddNewUser(newUser);
    }
}

//saves the current content of the leaderboard into the file
//recommended to do as soon as something changes in order to prevent user interference
static void SaveLeaderboard()
{
    FILE* file = fopen(LEADERBOARD_NAME, "w");

    if (file == NULL)
        return;

    UserData* current = leaderboard;
    while (current != NULL)
    {        
        fprintf(file, "%s, %d, %d\n", current->name, current->wins, current->games);
        
        current = current->next;
    }
    fclose(file);
}

//frees the memory of the leaderboard
void FreeLeaderboard()
{
    UserData* current = leaderboard;
    while (current != NULL)
    {
        UserData* next = current->next;
        free(current);
        current = next;
    }
}

//increases the number of games started by the current user
//saves to file immediately
void RegisterGame()
{
    if (currentUser == NULL)
        return;

    currentUser->games++;
    AdjustUserOrder(currentUser);
    SaveLeaderboard();
}

//increases the number of wins of the current user
//saves to file immediatly
void RegisterWin()
{
    if (currentUser == NULL)
        return;

    currentUser->wins++;
    AdjustUserOrder(currentUser);
    SaveLeaderboard();
}