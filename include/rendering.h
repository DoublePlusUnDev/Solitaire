#ifndef RENDERING_H
#define RENDERING_H

//initializes curses according to the render and color settings
void InitRenderer();

//draws the table of cards
void Render(Table table, CardPosition cursorPosition, CardPosition selectedPosition);

//draws a card pile at the given position
void RenderPile(CardPile pile, int x, int y, int visibleCards, int cursorIndex, int selectedIndex);

#endif