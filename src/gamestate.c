#include <stdio.h>
#include <stdbool.h>

#include "utils.h"
#include "logging.h"
#include "cards.h"
#include "gamestate.h"

#include "debugmalloc.h"

Table SetupTable()
{
    //allocate memory for the card pile matrix
    Table table;
    table.rows = 2;
    table.columns = 7;
    CardPile** piles = malloc(table.rows* sizeof(CardPile*));
    for (int i = 0; i < table.rows; i++)
    {
        piles[i] = malloc(table.columns * sizeof(CardPile));
        for (int j = 0; j < table.columns; j++)
        {
            piles[i][j] = CreateCardPile();
        }
    }

    //CardPile 
    Card cards[52];
    int i = 0;
    for (int suit = 1; suit <= 4; suit++)
    {
        for (int rank = 1; rank <= 13; rank++)
        {
            cards[i] = (Card){.suit = suit, .rank = rank};
            i++;
        }
    }
    
    ShuffleCards(cards, 52);

    //turn pile
    ResizePile(&piles[0][0], 24);
    piles[0][0].maxDisplay = 1;
    piles[0][0].uncovered = 255;
    piles[0][1].maxDisplay = 1;
    piles[0][2].maxDisplay = 0;
    for (int i = 3; i < 7; i++)
        piles[0][i].maxDisplay = 1;

    LOG("CARDS IN DRAW PILE:\n");
    for (i = 0; i < 24; i++)
    {
        AddCard(&piles[0][0], cards[i]); 
        LOG_ARGS("%d: %c%s\n", i + 1, CardRank(piles[0][0].cards[i]), CardSuit(piles[0][0].cards[i]));
    }

    //bottom rows
    i = 24;
    for (int column = 0; column < table.columns; column++)
    {
        LOG_ARGS("CARDS IN BOTTOM PILE %d:\n", column + 1);
        piles[1][column].uncovered = column;
        for (int n = 0; n < column + 1; n++)
        {
            AddCard(&piles[1][column], cards[i]);
            LOG_ARGS("%d: %c%s\n", n + 1, CardRank(piles[1][column].cards[n]), CardSuit(piles[1][column].cards[n]));
            i++;
        }
    }

    table.piles = piles;
    return table;
}

void FreeTable(Table table)
{
    for (int row = 0; row < table.rows; row++)
    {
        for (int column = 0; column < table.columns; column++)
        {
            //free the cards of each pile
            free(table.piles[row][column].cards);
        }
        //free the pointers to the rows
        free(table.piles[row]);
    }
    //free the pointers to the columns of rows
    free(table.piles);
}

CardPosition DefaultPosition(Table table)
{
    return (CardPosition){0, 0, table.piles[0][0].size - 1};
}

bool CanSelect(Table table, CardPosition position)
{
    //out of bounds
    if (position.row >= table.rows || 
        position.column >= table.columns || 
        position.inPile >= GetPile(table, position).size)
        return false;

    if (GetPile(table, position).size <= 0 || position.inPile < GetPile(table, position).uncovered)
        return false;

    CardPile pile = GetPile(table, position);
    for (int i = position.inPile; i < pile.size - 1; i++)
    {
        Card current = pile.cards[i];
        Card next = pile.cards[i + 1];

        //checks if next card is exactly one lesser in rank and different color, if not evalutes to false
        if (current.rank - 1 != next.rank || current.suit % 2 == next.suit % 2)
            return false;
    }

    return true;
}

//draws a card from the draw pile
//returns true if draw was successful
static bool DrawCard(Table table)
{
    CardPile* drawPile = &table.piles[0][0];
    CardPile* discardPile = &table.piles[0][1];
    
    //return if drawpile is empty
    if (drawPile->size == 0) return false;

    AddCard(discardPile, drawPile->cards[drawPile->size - 1]);
    RemoveTopCard(drawPile);
    return true;
}

//returns true if at least one card was reset
//puts all cards from discardpile back to the drawpile
static bool ResetDrawPile(Table table)
{
    CardPile* drawPile = &table.piles[0][0];
    CardPile* discardPile = &table.piles[0][1];

    if (discardPile->size == 0)
        return false;

    while(discardPile->size > 0)
    {
        AddCard(drawPile, discardPile->cards[discardPile->size - 1]);
        RemoveTopCard(discardPile);
        
    }
    
    return true;
}

bool TryInteract(Table table, CardPosition position)
{
    CardPile pile = GetPile(table, position);

    //top left piles, drawpiles
    if (position.row == 0 && position.column == 0)
    {
        if (pile.size > 0)
            return DrawCard(table);
        else
            return ResetDrawPile(table);
    }
    //if pile next to draw pile is empty draw
    else if (position.row == 0 && position.column == 1 && pile.size == 0)
    {
        return DrawCard(table);
    }

    return false;
}

bool CanMove(Table table, CardPosition from, CardPosition to)
{
    CardPile toPile = GetPile(table, to);
    CardPile fromPile = GetPile(table, from);
    Card fromCard = GetCard(table, from);
    Card toCard = GetCard(table, to);   

    //moving to upper row four piles when empty
    if (to.row == 0 && toPile.size == 0 && 3 <= to.column && to.column < 7)
        return fromCard.rank == 1;

    //moving to upper row four piles when not empty
    else if (to.row == 0 && 3 <= to.column && to.column < 7)
    {    //check if selected card is on top of pile and of same suit and one larger than the destination
        return (from.inPile == fromPile.size - 1) && (fromCard.suit == toCard.suit) && (fromCard.rank == toCard.rank + 1);
    }
    //moving to lower row into empty pile, must be king
    else if (to.row == 1 && toPile.size == 0)
        return fromCard.rank == 13;
    //moving to lower row non empty pile, must be one smaller and of different color
    else if (to.row == 1)
        return (fromCard.suit % 2) != (toCard.suit % 2) && (fromCard.rank + 1 == toCard.rank);

    return false;
}

void MoveCards(Table table, CardPosition from, CardPosition to)
{
    CardPile* fromPile = &table.piles[from.row][from.column];
    CardPile* toPile = &table.piles[to.row][to.column];

    for (int i = from.inPile; i < fromPile->size; i++)
    {
        AddCard(toPile, fromPile->cards[i]);
    }

    int cardsToRemove = fromPile->size - from.inPile;
    for (int i = 0; i < cardsToRemove; i++)
    {
        RemoveTopCard(fromPile);
    }

    //uncover top card if not already uncovered
    fromPile->uncovered = MIN(fromPile->uncovered, fromPile->size - 1);
}

bool HasWon(Table table)
{
    for (int row = 0; row < table.rows; row++)
    {
        for (int column = 0; column < table.columns; column++)
        {
            CardPile pile = table.piles[row][column];

            //four upper piles are full
            if (row == 0 && 3 <= column && column < 7)
            {
                if (pile.size != 13) 
                    return false;
            }

            //cards in any other pile
            else
            {
                if (pile.size != 0) return false;
            }
        }
    }

    return true;
}