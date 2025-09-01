#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <stdlib.h>

#include "utils.h"
#include "layout.h"
#include "debugmalloc.h"
#include "cards.h"

char* CardSuit(Card card)
{
    //returns UTF character, approach with caution!
    switch (card.suit)
    {
        case 1:
            return "♠";
        case 2:
            return "♥";
        case 3:
            return "♣";
        case 4:
            return "♦";
        //error handling, zero chars for padding
        default:
            return "E";
    }
}

char CardRank(Card card)
{
    if (2 <= card.rank && card.rank <= 9)
        return '0' + card.rank; //converts the number to the correcponding character

    switch (card.rank)
    {
        case 10:
            return '0';
        case 11:
            return 'J';
        case 12:
            return 'Q';
        case 13:
            return 'K';
        case 1:
            return 'A';
    
        //error handling
        default:
            return 'E';
    }
}

int CardColor(Card card)
{
    if (1 <= card.suit && card.suit <= 4)
        return card.suit;
    else
        //error handling
        return 0;
}

CardPile CreateCardPile()
{
    CardPile pile;
    pile.cards = (Card*)malloc(1 * sizeof(Card));
    pile.size = 0;
    pile.capacity = 1;

    pile.maxDisplay = 255;
    pile.uncovered = 0;
    return pile;
}

void ResizePile(CardPile* pile, int newCapacity)
{
    Card* newCards = (Card*)malloc(newCapacity * sizeof(Card));
    for (int i = 0; i < MIN(pile->size, newCapacity); i++)
    {
        newCards[i] = pile->cards[i];
    }
    free(pile->cards);

    pile->cards = newCards;
    pile->capacity = newCapacity;
}

void AddCard(CardPile* pile, Card card)
{
    if (pile->size == pile->capacity)
    {
        ResizePile(pile, 2 * pile->capacity);
    }
    
    pile->cards[pile->size] = card;
    pile->size++;
}

void RemoveTopCard(CardPile* pile)
{
    pile->size--;

    if (pile->size <= pile->capacity / 2 && pile->capacity != 1)
    {
        ResizePile(pile, pile->capacity / 2);
    }
}

void ShuffleCards(Card* cards, int length)
{
    if (length > 1)
    {
        for(int i = 0; i < length; i++)
        {
            int swap = rand() % length;

            Card temp = cards[i];
            cards[i] = cards[swap];
            cards[swap] = temp;
        }
    }
}

int DisplayedCardCount(CardPile pile)
{
    return MAX(MIN(pile.maxDisplay, pile.size), 1);
}

int RowHeight(Table table, int row)
{
    int maxHeight = 0;
    for (int i = 0; i < table.columns; i++)
    {
        maxHeight = MAX(CARD_HEIGHT + DisplayedCardCount(table.piles[row][i]) - 1, maxHeight);
    }

    return maxHeight;
}

CardPile GetPile(Table table, CardPosition position)
{
    return table.piles[position.row][position.column];
}

Card GetCard(Table table, CardPosition position)
{
    return GetPile(table, position).cards[position.inPile];
}