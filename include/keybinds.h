#ifndef KEYBINDS_H
#define KEYBINDS_H

#define UP KEY_UP
#define UP_ALT 'w'
#define RIGHT KEY_RIGHT
#define RIGHT_ALT 'd'
#define LEFT KEY_LEFT
#define LEFT_ALT 'a'
#define DOWN KEY_DOWN
#define DOWN_ALT 's'
#define INTERACT 32
#define INTERACT_ALT KEY_BACKSPACE
#define RESTART 'r'
#define RESTART_ALT KEY_ENTER
#define ESCAPE 27
#define ESCAPE_ALT 27

//converts alt inputs to normal ones
int ConvertInput(int input)
{
    if (input == UP || input == UP_ALT)
        return UP; 
    else if (input == RIGHT || input == RIGHT_ALT)
        return RIGHT;
    else if (input == LEFT || input == LEFT_ALT)
        return LEFT;
    else if (input == DOWN || input == DOWN_ALT)
        return DOWN;
    else if (input == INTERACT || input == INTERACT_ALT)
        return INTERACT;
    else if (input == RESTART || input == RESTART_ALT)
        return RESTART;
    else if(input == ESCAPE || input == ESCAPE_ALT)
        return ESCAPE;
    else if (input == KEY_MOUSE)
        return KEY_MOUSE;
    else
        return 0;
}

#endif