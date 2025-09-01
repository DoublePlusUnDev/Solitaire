#ifndef INPUT_H
#define INPUT_H

//initializes curses to be able to read in input
void InitInput();

//what to do after reading input
typedef enum InputResult
{
    NOTHING,
    RERENDER,
    EXIT,
    RESTART_GAME,
    WON
} InputResult;

//takes input from curses and modifies higlighted position, selected position and cards on the table accordingly
InputResult HandleInput(Table table, CardPosition* cursorPosition, CardPosition* selectedPosition);

#endif