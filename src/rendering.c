#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "layout.h"
#include "colors.h"
#include "cards.h"

//sets up the curses renderer for the game
void InitRenderer()
{
    resize_term(SCREEN_HEIGHT, SCREEN_WIDTH);
    start_color();

    CreateColorPairs();
}

//prints a & encoded colored text
//&FS the First letter is the background color c: card background passed by argument b: default background 
//the Second letter is the text color t: normal, 1-4:corresponding suit color
void PrintInColor(char* str, int cardBackground)
{
    int i = 0;
    int length = strlen(str);
    while (i < length)
    {
        //color indicator character & followed by fore and background color chars
        if(str[i] == '&' && i + 2 < length)
        {
            int isValid = true;

            //add foreground color
            int color = 0;
            if (str[i + 2] == 't')
                color = 0;
            else if (str[i + 2] == '1')
                color = 1;
            else if (str[i + 2] == '2')
                color = 2;
            else if (str[i + 2] == '3')
                color = 3;
            else if (str[i + 2] == '4')
                color = 4;
            else
                isValid = false;

            //add background color
            if (str[i + 1] == 'b')
                color += BACKGROUND;
            else if (str[i + 1] == 'c')
                color += cardBackground;
            else
                isValid = false;
                
            //set attribute and jump to next character that's not part of the color encoding
            if (isValid)
                attrset(COLOR_PAIR(color));
            i += 3;
            
        }
        else
        {
            printw("%c", str[i]);
            i++;
        }
    }
}

//renders a singular pile to the screen, position of the top left corner at pos x, y
void RenderPile(CardPile pile, int x, int y, int cursorIndex, int selectedIndex)
{
    if (pile.maxDisplay == 0) return;

    //number of cards visible
    int visibleCards = DisplayedCardCount(pile);
    //probaly will fit even with UTF chars, if not increase the buffer size
    char formattedLine[SCREEN_WIDTH];

    for (int i = 0; i < CARD_HEIGHT + (visibleCards - 1); i++)
    {
        int cardIndex = MAX(MIN(pile.size - 1 - (visibleCards - i), pile.size - 1), 0);
        
        int cardBackground;
        //handle background coloring
        //the card where the cursor is
        if (cursorIndex != -1 && cardIndex == cursorIndex)
            cardBackground = CARD_CURSOR;
        
        //selected card and cards below it
        else if (selectedIndex != -1 && cardIndex >= selectedIndex)
            cardBackground = CARD_SELECTED;
        
        //empty pile color
        else if (pile.size == 0)
            cardBackground = BACKGROUND;
        
        else
            cardBackground = CARD_NORMAL;
              
        //line on top of card
        if (i == 0)
        {
            move(y, x);
            PrintInColor(" _______ ", cardBackground);
        }
        //visible but partially hidden cards
        else if (i < visibleCards)
        {
            Card card = pile.cards[cardIndex];
            move(y + i, x);
            if (cardIndex >= pile.uncovered)
            {
                char* line = "|&c%d%c&ct_____&c%d%s&bt|";
                sprintf(formattedLine, line, CardColor(card), CardRank(card), CardColor(card), CardSuit(card));
                PrintInColor(formattedLine, cardBackground);
            }
            else
                PrintInColor("|&ct\\_____/&bt|", cardBackground);
        }
        //bottom card
        else
        {
            Card card = pile.cards[cardIndex];

            move(y + i, x);
            //first line of bottom part
            if (i - visibleCards == 0)
            {
                if (pile.size == 0)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else if (cardIndex >= pile.uncovered)
                {
                    char* line = "|&c%d%c&ct     &c%d%s&bt|";
                    sprintf(formattedLine, line, CardColor(card), CardRank(card), CardColor(card), CardSuit(card));
                    PrintInColor(formattedLine, cardBackground);
                }
                else
                    PrintInColor("|&ct\\     /&bt|", cardBackground);
            }
            //second line of bottom part
            else if (i - visibleCards == 1)
            {
                if (pile.size == 0)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else if (cardIndex >= pile.uncovered)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else
                    PrintInColor("|&ct \\   / &bt|", cardBackground);
            }
            //third line of bottom part
            else if (i - visibleCards == 2)
            {
                if (pile.size == 0)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else if (cardIndex >= pile.uncovered)
                {
                    char* line = "|&ct   &c%d%c%s  &bt|";
                    sprintf(formattedLine, line, CardColor(card), CardRank(card), CardSuit(card));
                    PrintInColor(formattedLine, cardBackground);
                }
                else
                    PrintInColor("|&ct  | |  &bt|", cardBackground);
            } 
            //fourth line if bottom part
            else if (i - visibleCards == 3)
            {
                if (pile.size == 0)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else if (cardIndex >= pile.uncovered)
                    PrintInColor("|&ct       &bt|", cardBackground);
                else
                    PrintInColor("|&ct /   \\ &bt|", cardBackground);
            }
            //last line of bottom part
            else if (i - visibleCards == 4)
            {
                if (pile.size == 0)
                    PrintInColor("|&ct_______&bt|", cardBackground);
                else if (cardIndex >= pile.uncovered)
                {
                    char* line = "|&c%d%s&ct_____&c%d%c&bt|"; 
                    sprintf(formattedLine, line, CardColor(card), CardSuit(card), CardColor(card), CardRank(card));
                    PrintInColor(formattedLine, cardBackground);
                }
                else
                    PrintInColor("|&ct/_____\\&bt|", cardBackground);
            }
            move(y + i, 12);
        }
    }
}

//renders the grid of piles to the screen
void Render(Table table, CardPosition cursorPosition, CardPosition selectedPosition)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        mvchgat(i, 0, -1, 0, BACKGROUND, NULL);
    }    

    move(0, 0);
    attrset(COLOR_PAIR(BACKGROUND));
    int yOffset = ROW_OFFSET;
    for (int row = 0; row < table.rows; row++)
    {
        for (int column = 0; column < table.columns; column++)
        {
            //checks if current pile has the cursor
            int cursorIndex = -1;
            if (row == cursorPosition.row && column == cursorPosition.column)
                cursorIndex = cursorPosition.inPile;
            
            //checks if current pile is selected
            int selectedIndex = -1;
            if (row == selectedPosition.row && column == selectedPosition.column)
                selectedIndex = selectedPosition.inPile;

            RenderPile(table.piles[row][column], COLUMN_OFFSET + (CARD_WIDTH + COLUMN_SPACE) * column, yOffset, cursorIndex, selectedIndex);            
        }
        yOffset += RowHeight(table, row) + ROW_SPACE;
    }
}