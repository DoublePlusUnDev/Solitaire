//IF YOU WANT TO USE MOUSE INPUT YOU NEED TO DISABLE QUICK EDIT MODE IN YOUR WINDOWS TERMINAL
#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "logging.h"
#include "cards.h"
#include "rendering.h"
#include "gamestate.h"
#include "input.h"
#include "leaderboard.h"

#include "debugmalloc.h"

void InitCurses()
{
    //fixes utf encoding issues
    setlocale(LC_ALL, "en_US.UTF-8");

    initscr();
    
    InitRenderer();
    InitInput();
}

void EndCurses()
{
    endwin();
}

void GameLoop()
{
    Table table = SetupTable();
    CardPosition cursorPosition = DefaultPosition(table);
    CardPosition selectedPosition = INVALID_POSITION;
    LOG("INITIALIZED TABLE\n");
    LOG("STARTED GAME\n");

    bool shouldRefresh = true;
    RegisterGame();
    while (true)
    {
        if (shouldRefresh)
        {
            clear();
            Render(table, cursorPosition, selectedPosition);
            refresh();
            shouldRefresh = false;
        }

        InputResult result = HandleInput(table, &cursorPosition, &selectedPosition);
        
        if (result == RERENDER)
            shouldRefresh = true;
        else if (result == EXIT)
        {
            LOG("USER EXITED THE GAME\n");
            break;
        }
        else if (result == WON)
        {
            LOG("USER WON THE GAME\n");
            RegisterWin();
            break;
        }
        else if (result == RESTART_GAME)
        {
            FreeTable(table);

            table = SetupTable();
            cursorPosition = DefaultPosition(table);
            selectedPosition = INVALID_POSITION;

            shouldRefresh = true;
            RegisterGame();
            LOG("GAME WAS RESTARTED BY USER\n");
        }
    }

    LOG("GAME HAS BEEN ENDED\n");
    FreeTable(table);
}

int main(void)
{
    StartLog();
    InitCurses();
    LoadLeaderboard();

    srand(time(NULL));
    LOG("INITIALIZED GAME\n");

    while (true)
    {   
        bool userNameAttempt = ReadUserName();

        //succesfully read in username
        if (userNameAttempt)
            GameLoop();
        //failed user name routine 
        else
            break;
    }

    LOG("PREPARING TO SHUT DOWN...\n");
    
    EndCurses();
    FreeLeaderboard();
    CloseLog();
    return 0;
}