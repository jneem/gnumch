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
#include <Player.h>
#include <Game.h>
#include <Animation.h>

extern Game *game;

Player::Player ():
    anim(NULL)
{
    eat_offense = 1;
    eat_defense_live = 1;
    eat_defense_eatback = 0;

    moving = appearing = disappearing = false;
}

Player::~Player ()
{
}

int Player::attacked(int attacker_offense)
{
    if( eat_defense_eatback > attacker_offense ) {
        return -1;
    } else if( eat_defense_live >= attacker_offense) {
        return 0;
    }
    return 1;
}

Point Player::getPos()
{
    Point ret(x, y);
    return ret;
}

SDL_Surface *Player::getPic(int pic_x, int pic_y, int w, int h,
                            SDL_Rect *src, SDL_Rect *dest)
{
    src->x = src->y = dest->x = dest->y = 0;
    src->w = w;
    src->h = h;
    if(!moving) {
        if(pic_x != x || pic_y != y)
            printWarning("tried to get a picture at invalid coordinates\n");
    } else {
        if(!(pic_x == x && pic_y == y  ||  pic_x == old_x && pic_y == old_y)) {
            printWarning("tried to get a picture at invalid coordinates\n");
            pic_x = x, pic_y = y;
        }

        double pct_moved = ((double)SDL_GetTicks() - action_start)
                            / game->getChangeTime();
        int from_x, from_y;

        if(pct_moved < 0) pct_moved = 0;
        if(pct_moved > 1) pct_moved = 1;
        if(pic_x == x && pic_y == y) {
            from_x = old_x;
            from_y = old_y;
            pct_moved = 1 - pct_moved;
        } else {
            from_x = x;
            from_y = y;
        }

        if(pic_x > from_x) {                // moving right
            src->x = (int)(pct_moved * w);
            src->w = w - src->x;
        } else if(pic_x < from_x) {         // moving left
            dest->x = (int)(pct_moved * w);
            src->w  = w - dest->x;
        } else if(pic_y > from_y) {         // moving down
            src->y = (int)(pct_moved * h);
            src->h = h - src->y;
        } else if(pic_y < from_y) {         // moving up
            dest->y = (int)(pct_moved * h);
            src->h  = h - dest->y;
        }
    }
    return anim.getFrame();
}

SDL_Surface *Player::getWholePic(int w, int h, SDL_Rect *dest)
{
    SDL_Rect dummy;
    SDL_Surface *ret;

    // make sure we choose a square containing our top left coordinates
    if (old_x < x || old_y < y) {
        ret = getPic(old_x, old_y, 1, 1, &dummy, dest);
        dest->x -= (x-old_x) * w;
        dest->y -= (y-old_y) * h;
    } else {
        ret = getPic(x, y, 1, 1, &dummy, dest);
    }

    return ret;
}

bool Player::exist()
{
    return exists;
}

bool Player::isIdle()
{
    return exists && !(moving || appearing || disappearing);
}

void Player::delay(int delay)
{
    action_start += delay;
}

bool Player::isAt(int a, int b)
{
    return exists && !moving && x == a && y == b;
}

bool Player::isNear(int a, int b)
{
    return isAt(a, b) || (exists && moving && ((old_x == a && old_y == b)
                                           ||  (x == a && y == b))
                         );
}

void Player::spawn(int time)
{
    exists = true;
}

void Player::die(int time)
{
    anim.setState(ANIM_DISAPPEARING);
    disappearing = true;
    appearing = false;
}

void Player::updatePos()
{
    int now = SDL_GetTicks();

    if(moving) {
        if(now >= action_start + game->getChangeTime()) {
            /* stop the motion */
            printMsg(3, "motion stopped: (%d,%d)\n", x, y);
            moving = 0;
            anim.setState(ANIM_NORMAL);
            action_start = now;
            game->playerStop( this, x, y, now );
        } else {
            anim.nextFrame();
        }

        game->setDirty(x, y);
        game->setDirty(old_x, old_y);

    } else if(anim.getState() == ANIM_EATING) {
        if(now >= action_start + game->getEatTime()) {
            /* stop eating */
            printMsg(3, "eating stopped\n");
            anim.setState(ANIM_NORMAL);
            game->setDirty(x, y);
        } else if(anim.nextFrame()) {
            game->setDirty(x, y);
        }
    } else if (anim.finished()) {
        if (appearing) {
            assert( !moving && !disappearing );
            anim.setState(ANIM_NORMAL);
            game->setDirty(x, y);
            appearing = false;
        } else if (disappearing) {
            assert (disappearing && !moving && !appearing );
            game->setDirty(x, y);
            exists = false;
            disappearing = false;
        }
    } else if (anim.nextFrame()) {
        game->setDirty(x, y);
    }
}

void Player::move(int old_x, int old_y, int new_x, int new_y, int time)
{
    if(!exists) return;

    moving = true;
    if(new_x > old_x)
    {
        anim.setDir(DIR_RIGHT);
    }
    else if(new_x < old_x)
    {
        anim.setDir(DIR_LEFT);
    }
    else if(new_y > old_y)
    {
        anim.setDir(DIR_DOWN);
    }
    else if(new_y < old_y)
    {
        anim.setDir(DIR_UP);
    }
    else    // teleporting, not walking
    {
        moving = false;
        disappearing = false;
        appearing = true;
        anim.setDir(DIR_RIGHT);
        anim.setState(ANIM_APPEARING);
    }

    if (moving)
        anim.setState(ANIM_WALKING);

    printMsg(2, "player moving to (%d,%d)\n", new_x, new_y);
    action_start    = time;
    this->old_x     = old_x;
    this->old_y     = old_y;
    this->x         = new_x;
    this->y         = new_y;
    game->setDirty(x, y);
}

void Player::munch(int time)
{
    anim.setState(ANIM_EATING);
    moving       = 0;
    action_start = time;
}
