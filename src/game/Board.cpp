/*
 *  Copyright (C) 2005 Joe Neeman <spuzzzzzzz@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <Board.h>
#include <Game.h>
#include <Level.h>
#include <Player.h>

extern Game *game;

Board::Board(int w, int h, const vector<Player*> &players)
{
    int i, j;

    width = w;
    height = h;
    goodies = 0;
    paused = 0;

    this->players = players;

    /* set up the Squares */
    square = (Square***)malloc(sizeof(void*) * (w*h + w));
    Square **layer1 = (Square**)(square + w);
    for(i=0; i<width; i++) {
        square[i] = layer1;
        layer1  += height;
        for(j=0; j<height; j++) {
            square[i][j] = new Square();
        }
    }

    /* the dirty array */
    dirty = (bool**)malloc(sizeof(void*) * w + sizeof(bool) * w*h);
    bool *layer = (bool*)(dirty + w);
    for(i=0; i<width; i++) {
        dirty[i] = layer;
        layer += height;
    }
}

Board::~Board()
{
    int i, j;

    for(i=0; i<width; i++) {
        for(j=0; j<height; j++) {
            delete square[i][j];
        }
    }
    free(square);
    free(dirty);
}

int Board::getHeight()
{
    return height;
}

int Board::getWidth()
{
    return width;
}

void Board::setDirty(int x, int y)
{
    if(x < 0 || x >= width || y < 0 || y >= height)
        return;
    dirty[x][y] = 1;
}

void Board::redraw()
{
    int i, j;

    for(i=0; i<width; i++) {
        for(j=0; j<height; j++) {
            if(dirty[i][j]) {
                game->redrawSquare(i, j);
                dirty[i][j] = 0;
            }
        }
    }
}

/* 1) refill the board with Number*s
 * 2) spawn the Muncher
 * 3) set the troggle spawn times
 */

void Board::reset()
{
    /* fill the board with numbers */
    int i, j;

    goodies = 0;
    for(i=0; i<width; i++) {
        for(j=0; j<height; j++) {
            game->setNum(i, j, game->randomNumber(), false);
            dirty[i][j] = 0; // zero because we count on Game::redrawAll()
                             // to redraw things efficiently
        }
    }
}

void Board::unset()
{
    int i, j;

    for(i=0; i<width; i++) {
        for(j=0; j<height; j++) {
            game->setNum(i, j, NULL, false);
        }
    }
}

void Board::update()
{
    int i;

    for(i=0; i<(int)players.size(); i++) {
        if(!paused) players[i]->update();
    }
    redraw();
}

int Board::munch(int x, int y)
{
    if(!square[x][y]->filled()) {
        return 0;
    } else if(!square[x][y]->good()) {
        return -1;
    }
    return 1;
}

void Board::pause()
{
    paused = SDL_GetTicks();
}

void Board::resume()
{
    int now = SDL_GetTicks();
    int delay = now - paused;
    int i;

    for(i=0; i<(int)players.size(); i++) {
        players[i]->delay(delay);
    }
    paused = 0;
}

vector<Player*> Board::getPlayersAt(int x, int y)
{
    vector<Player*> ret;

    for(int i=0; i<(int)players.size(); i++) {
        if(players[i]->isAt(x, y))
            ret.push_back( players[i] );
    }
    return ret;
}

vector<Player*> Board::getPlayersNear(int x, int y)
{
    vector<Player*> ret;

    for(int i=0; i<(int)players.size(); i++) {
        if(players[i]->isNear(x, y))
            ret.push_back( players[i] );
    }
    return ret;
}

/* square query functions */
bool Board::filled(int x, int y)
{
    return square[x][y]->filled();
}

bool Board::good(int x, int y)
{
    return square[x][y]->good();
}

void Board::setNum(int x, int y, Number *num, bool wincheck)
{
    if( !contains(x, y) )
        return;

    goodies += (num && num->good()) - 
               (square[x][y]->filled() && square[x][y]->good());
    printMsg(1, "number changing at (%d,%d), goodies=%d\n", x, y, goodies);
    if(!goodies && wincheck) {
        game->win();
    }
    square[x][y]->setNum(num);
    dirty[x][y] = 1;
}

Number *Board::getNum(int x, int y)
{
    return square[x][y]->getNum();
}

SDL_Surface *Board::getTextPic(int x, int y)
{
    return square[x][y]->getTextPic();
}

/*__________________________________squares___________________________________*/

Square::Square()
{
    num = NULL;
}

Square::~Square()
{
}

bool Square::filled()
{
    return num;
}

bool Square::good()
{
    return (!num) || num->good();
}

void Square::setNum(Number *numb)
{
    num = numb;
}

Number *Square::getNum()
{
    return num;
}

SDL_Surface *Square::getTextPic()
{
    if(!num) return NULL;
    return num->getPic();
}

