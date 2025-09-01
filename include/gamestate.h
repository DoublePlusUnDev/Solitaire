#ifndef GAMESTATE_H
#define GAMESTATE_H

//sets up the table for klondike solitaire
Table SetupTable();

//frees the memory of the table
void FreeTable(Table table);

//returns the starting position for the highlighted card
CardPosition DefaultPosition(Table table);

//can the given position be legally selected
bool CanSelect(Table table, CardPosition position);

//tries quick interaction at given position
bool TryInteract(Table table, CardPosition position);

//can cards legally be moved from "from" position to "to" position
bool CanMove(Table table, CardPosition from, CardPosition to);

//move cards from "from" position to "to" position
void MoveCards(Table table, CardPosition from, CardPosition to);

//detects if game has been won
bool HasWon(Table table);

#endif