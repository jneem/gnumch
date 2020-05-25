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
#include <Muncher.h>
#include <Animation.h>
#include <Game.h>
#include <Board.h>
#include <Event.h>

extern Game *game;

Muncher::Muncher(Animation *new_anim)
{
    anim.setAnim(new_anim);
    x = y = old_x = old_y = -1;
    moving = 0;
    action_start = 0;
    exists = 0;
    score = 0;
}

Muncher::~Muncher()
{
}

void Muncher::spawn(int time)
{
    Player::spawn(time);
    printMsg(1, "muncher spawning\n");
    key_queue.clear();
    game->playerMove( this, 0, 0, 0, 0, time );
}

void Muncher::handleKey(enum Key key)
{
    key_queue.push_back(key);
}

void Muncher::givePoints(int p)
{
    score += p;
    game->updateScore( this, score );
}

void Muncher::update()
{
    int now = SDL_GetTicks();
    if ( isIdle() && !(disappearing || appearing) && !key_queue.empty() ) {
        enum Key key = key_queue.front();
        key_queue.pop_front();
        printMsg(2, "muncher got key %d\n", key);

        switch( key )
        {
            case KEY_UP:
                if( y > 0 )
                    game->playerMove( this, x, y, x, y-1, now );
                break;
            case KEY_DOWN:
                if( y < game->getHeight() - 1 )
                    game->playerMove( this, x, y, x, y+1, now );
                break;
            case KEY_LEFT:
                if( x > 0 )
                    game->playerMove( this, x, y, x-1, y, now );
                break;
            case KEY_RIGHT:
                if( x < game->getWidth() - 1 )
                    game->playerMove( this, x, y, x+1, y, now );
                break;
            case KEY_MUNCH:
                game->playerMunch( this, x, y, now );
                break;
            case KEY_SPAWN:
            case KEY_MENU:
            case KEY_NUM:   // just to keep the compiler happy
                break;
        }
    }
    while (!exists && !key_queue.empty()) {
        enum Key key = key_queue.front();
        key_queue.pop_front();

        if (key == KEY_SPAWN) {
            game->playerSpawn( this, now );
        }
    }
    if( exists )
    {
        updatePos();
    }
}
