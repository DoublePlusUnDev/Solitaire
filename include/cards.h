#ifndef CARDS_H
#define CARDS_H

//for storing a single card
typedef struct Card
{
    int suit;   //suit of card: ♠(black), ♥(red), ♣(black), ♦(red)
    int rank;   //rank of card: A, 2, 3, 4, 5, 6, 7, 8, 9, 0, J, Q, K
}
Card;

//gets the symbol of a card color, returned as string might be UTF-8 encoded
char* CardSuit(Card card);

//gets the character representation of the rank of a card
char CardRank(Card card);

//gets the index of the text color associated with the card
int CardColor(Card card);

//pile of cards with a dynamically allocated array
typedef struct CardPile
{
    Card* cards;    //card array
    int size;       //size of array
    int capacity;   //actual capacity of array

    int maxDisplay; //top n cards to be rendered
    int uncovered;  //card up to n index will be face down(n will be face up)
} CardPile;

//intializes an empty pile of cards with default properties
CardPile CreateCardPile();

//resizes a pile to the given capacity and then cleans up it's own mess
void ResizePile(CardPile* pile, int newCapacity);

//adds a single card to a pile, doubles capacity if pile is full
void AddCard(CardPile* pile, Card card);

//removes the top card from a pile and reduces pile size if at less than half the capacity
void RemoveTopCard(CardPile* pile);

//shuffles an array of cards
void ShuffleCards(Card* cards, int length);

//displays the number cards to be rendered for a given pile, will be one for empty piles
int DisplayedCardCount(CardPile pile);

//a row*column grid that stores the piles of playing cards on the table
typedef struct Table
{
    CardPile** piles;
    int rows;
    int columns;
} Table;

//the number of lines it takes to render a given row of the table
int RowHeight(Table table, int row);

//specifies a the position of a card at the table
typedef struct CardPosition
{
    int row;
    int column;
    int inPile;
} CardPosition;

//position outside the bounds of the table, used for unselecting pile
#define INVALID_POSITION (CardPosition){-1, -1, -1}

//returns the corresponding pile to a given position at the table
CardPile GetPile(Table table, CardPosition position);

//returns the corresponding card to a given position at the table
Card GetCard(Table table, CardPosition position);

#endif