#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <stdbool.h>

#include "utils.h"
#include "logging.h"
#include "layout.h"
#include "keybinds.h"
#include "cards.h"
#include "gamestate.h"
#include "input.h"

void InitInput()
{
    cbreak(); //disables input buffering
    noecho(); //input isn't automatically displayed when typed

    curs_set(0); //disables the flashing cursor
    keypad(stdscr, true);
    mousemask(BUTTON1_PRESSED | REPORT_MOUSE_POSITION, NULL);
}

static CardPosition GetClickPosition(Table table, int x, int y)
{
    int column = (x - COLUMN_OFFSET) / (CARD_WIDTH + COLUMN_SPACE);
    int relativeX = (x - COLUMN_OFFSET) % (CARD_WIDTH + COLUMN_SPACE);

    int row = 0;
    int relativeY = y - ROW_OFFSET;
    //rows are not of uniform length so you need to loop through them one by one
    while (row < table.rows && relativeY >= RowHeight(table, row))
    {
        relativeY -= (RowHeight(table, row) + ROW_SPACE);
        row++;
    }

    //out of bound clicks
    if (relativeX < 0 || relativeY < 0 || column >= table.columns || row >= table.rows)
        return INVALID_POSITION;

    //edges of cards
    if (relativeX == 0 || relativeX >= CARD_WIDTH - 1 || relativeY == 0)
        return INVALID_POSITION;

    CardPile pile = table.piles[row][column];

    int inPile = 0;
    //pile slice seleceted
    if (relativeY < DisplayedCardCount(pile))
    {
        inPile = (relativeY - 1) + MAX(pile.size - DisplayedCardCount(pile), 0);
    }
    //bottom card
    else if (relativeY < DisplayedCardCount(pile) - 1 + CARD_HEIGHT)
    {
        inPile = MAX(pile.size - 1, 0);
    }
    //space below bottom card
    else
    {
        return INVALID_POSITION;
    }

    return (CardPosition){row, column, inPile};
}

static InputResult KeyMoveInput(Table table, CardPosition* cursorPosition, int input)
{
    CardPile pile = GetPile(table, *cursorPosition);

    int relativeCardPosition = cursorPosition->inPile - MAX(pile.size - DisplayedCardCount(pile), 0);

    if (input == UP)
    {
        //can move up inside pile
        relativeCardPosition--;
        if (relativeCardPosition >= 0)
        {
            cursorPosition->inPile = relativeCardPosition + (pile.size - DisplayedCardCount(pile));
            return RERENDER;
        }

        //search for a pile higher up
        int nextRow = cursorPosition->row - 1;
        while (nextRow >= 0)
        {
            //found suitable pile
            if (table.piles[nextRow][cursorPosition->column].maxDisplay > 0)
            {
                cursorPosition->row = nextRow;
                cursorPosition->inPile = MAX(GetPile(table, *cursorPosition).size - 1, 0);
                return RERENDER;
            }
            
            nextRow--;
        }
    }
    else if (input == DOWN)
    {
        //can move down inside pile
        relativeCardPosition++;
        if (relativeCardPosition < DisplayedCardCount(pile))
        {
            cursorPosition->inPile = relativeCardPosition + (pile.size - DisplayedCardCount(pile));
            return RERENDER;
        }

        //search for a pile higher up
        int nextRow = cursorPosition->row + 1;
        while (nextRow < table.rows)
        {
            //found suitable pile (pile has the capability to store cards even if empty)
            if (table.piles[nextRow][cursorPosition->column].maxDisplay > 0)
            {
                cursorPosition->row = nextRow;
                cursorPosition->inPile = MAX(GetPile(table, *cursorPosition).size - 1, 0);
                return RERENDER;
            }
            
            nextRow++;
        }
    }
    else if (input == RIGHT)
    {
        int nextColumn = cursorPosition->column + 1;
        while (nextColumn < table.columns)
        {
            if (table.piles[cursorPosition->row][nextColumn].maxDisplay > 0)
            {
                cursorPosition->column = nextColumn;
                cursorPosition->inPile = MAX(GetPile(table, *cursorPosition).size - 1, 0);
                return RERENDER;
            }

            nextColumn++;
        }
    }
    else if (input == LEFT)
    {
        int nextColumn = cursorPosition->column - 1;
        while (nextColumn >= 0)
        {
            if (table.piles[cursorPosition->row][nextColumn].maxDisplay > 0)
            {
                cursorPosition->column = nextColumn;
                cursorPosition->inPile = MAX(GetPile(table, *cursorPosition).size - 1, 0);
                return RERENDER;
            }
            
            nextColumn--;
        }
    }

    return NOTHING;
}

static InputResult InteractInput(Table table, CardPosition* cursorPosition, CardPosition* selectedPosition)
{
    //test if pile can be interacted directly
    if (TryInteract(table, *cursorPosition))
    {
        LOG_ARGS("INTERACTED WITH PILE AT row %d column %d\n", cursorPosition->row, cursorPosition->column);

        //set cursor card to the top of current pile
        cursorPosition->inPile = MAX(GetPile(table, *cursorPosition).size - 1, 0);
        
        //select current pile if possible
        if (CanSelect(table, *cursorPosition))
            *selectedPosition = *cursorPosition;
        else
            *selectedPosition = (CardPosition){-1, -1, -1};

        return RERENDER;
    }

    //if you have selected cards try to move them to another position
    if (selectedPosition->row != -1 && selectedPosition->column != -1 && selectedPosition->inPile != -1)
    {
        //checks if top of a pile is higlighted and if valid move
        if (cursorPosition->inPile == MAX(GetPile(table, *cursorPosition).size - 1, 0) && CanMove(table, *selectedPosition, *cursorPosition))
        {
            LOG_ARGS("MOVED %d CARD(S) FROM row %d column %d TO row %d column %d\n", GetPile(table, *selectedPosition).size - selectedPosition->inPile, selectedPosition->row + 1, selectedPosition->column + 1, cursorPosition->row + 1, cursorPosition->column + 1);

            MoveCards(table, *selectedPosition, *cursorPosition);

            //invalidate selection and put cursor to the top of pile
            *selectedPosition = (CardPosition){-1, -1, -1};
            cursorPosition->inPile = GetPile(table, *cursorPosition).size - 1;

            if (HasWon(table)) return WON;
            else return RERENDER;
        }
    }

    //check if position can be selected
    if (CanSelect(table, *cursorPosition))
    {
        *selectedPosition = *cursorPosition;
        return RERENDER;
    }

    return NOTHING;
}

static InputResult MouseInput(Table table, CardPosition* cursorPosition, CardPosition* selectedPosition, MEVENT mouseEvent)
{
    CardPosition clickPosition = GetClickPosition(table, mouseEvent.x, mouseEvent.y);
    
    //only clicks to valid positions and piles that can store cards
    if (clickPosition.row != - 1 && GetPile(table, clickPosition).maxDisplay > 0)
    {
        *cursorPosition = clickPosition;
        InteractInput(table, cursorPosition, selectedPosition);
    }

    if (HasWon(table)) return WON;
    else return RERENDER;
}

InputResult HandleInput(Table table, CardPosition* cursorPosition, CardPosition* selectedPosition)
{
    int input = getch();

    input = ConvertInput(input);
    MEVENT mouseEvent;

    if (input == KEY_MOUSE && getmouse(&mouseEvent) == OK)
    {
        return MouseInput(table, cursorPosition, selectedPosition, mouseEvent);
    }
    else if (input == UP || input == DOWN || input == RIGHT || input == LEFT)
    {
        return KeyMoveInput(table, cursorPosition, input);
    }
    else if(input == INTERACT)
    {
        return InteractInput(table, cursorPosition, selectedPosition);
    }
    else if (input == RESTART)
    {
        return RESTART_GAME;
    }
    else if (input == ESCAPE)
    {
        return EXIT;
    }
    
    return NOTHING;    
}