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
#include <Game.h>
#include <Board.h>
#include <Troggle.h>
#include <Animation.h>

extern Game *game;

/* a list of possible TroggleAction callbacks */
TrogActionDef action[] = {
    {"getMove_straight",    getMove_straight},
    {"getMove_random",      getMove_random},
    {"getMove_chase",       getMove_chase},
    {"getMove_run",         getMove_run},
    {"onMove_create",       onMove_create},
    {"onStop_munch",        onStop_munch}
};

TroggleDef::TroggleDef(Animation *anim_, TroggleAction get_move,
                       TroggleAction on_move=NULL, TroggleAction on_stop=NULL)
{
    anim = anim_;
    getMove = get_move;
    onMove = on_move;
    onStop = on_stop;
}

TroggleAction getAction(const char *name)
{
    int i;

    if(!strcmp(name, "none")) return NULL;
    for(i=0; i<(int)(sizeof(action)/sizeof(TrogActionDef)); i++) {
        if(!strcmp(action[i].name, name))
            return action[i].action;
    }
    printError("couldn't find a TroggleAction named %s\n", name);
    return NULL;
}

Troggle::Troggle()
{
    x = y = old_x = old_y = -1;
    moving = 0;
    action_start = 0;
    exists = 0;
    eat_offense = 3;
    eat_defense_live = eat_defense_eatback = 2;
}

void Troggle::spawn(int time)
{
    int width = game->getWidth();
    int height = game->getHeight();

    if(rand() % 2) {
        if(rand() % 2) {    /* spawn from above */
            old_y = -1;
            y = 0;
            anim.setDir(DIR_DOWN);
        } else {            /* spawn from below */
            old_y = height;
            y = height - 1;
            anim.setDir(DIR_UP);
        }
        old_x = x = rand() % width;
    } else {
        if(rand() % 2) {    /* spawn from the left */
            old_x = -1;
            x = 0;
            anim.setDir(DIR_RIGHT);
        } else {            /* spawn from the right */
            old_x = width;
            x = width - 1;
            anim.setDir(DIR_LEFT);
        }
        old_y = y = rand() % height;
    }
    exists = 1;
    game->playerMove( this, old_x, old_y, x, y, time );
}

void Troggle::queueSpawn()
{
    TroggleDef *def = reinterpret_cast<TroggleGame*>(game)->randomTroggle();

    anim.setAnim(def->anim);
    getMove = def->getMove;
    onMove  = def->onMove;
    onStop  = def->onStop;
}

void Troggle::die(int time)
{
    game->setDirty( x, y );
    game->troggleNextSpawn( this, time );
    anim.stopSound();
    exists = 0;
}

void Troggle::update()
{
    int now = SDL_GetTicks();

    if( isIdle() && now >= action_start + game->getTrogWait() )
    {
        calculateMove();
    }
    if( exists )
    {
        updatePos();
    }
}

/*________________________________Utility Functions___________________________*/

void Troggle::calculateMove()
{
    int old_x = this->x;
    int old_y = this->y;
    this->getMove(this);
    int new_x = this->x;
    int new_y = this->y;

    // return the Troggle to its previous state
    this->x = old_x, this->y = old_y;

    game->playerMove( this, old_x, old_y, new_x, new_y, SDL_GetTicks() );
}

/*_____________________________TroggleAction callbacks________________________*/

void getMove_straight(Troggle *trog)
{
    trog->x = 2*trog->x - trog->old_x;
    trog->y = 2*trog->y - trog->old_y;
}

void getMove_random(Troggle *trog)
{
    if(rand()%2) {
        trog->x = 2*trog->x - trog->old_x;
        trog->y = 2*trog->y - trog->old_y;
    } else {
        if(rand()%2) { /* turn left */
            int tmp = trog->x - (trog->y - trog->old_y);
            trog->y -= trog->x - trog->old_x;
            trog->x = tmp;
        } else { /* turn right */
            int tmp = trog->x + (trog->y - trog->old_y);
            trog->y += trog->x - trog->old_x;
            trog->x = tmp;
        }
    }
}

void getMove_chase(Troggle *trog)
{
    int dx, dy;
    int width = game->getWidth();
    int height = game->getHeight();
    Player *m = game->getNearestMuncher(trog->x, trog->y);

    if( m )
    {
        Point munch_pos = m->getPos();

        dx = trog->x - munch_pos.x;
        dy = trog->y - munch_pos.y;

        if(dx < 0)
            dx *= -1;
        if(dy < 0)
            dy *= -1;

        if( dx <= width/2 && dy <= height/2 )
        {
            if(dx > dy || (dx == dy && rand()%2))
            {
                trog->x += (trog->x - munch_pos.x > 0)? -1 : 1;
            }
            else
            {
                trog->y += (trog->y - munch_pos.y > 0)? -1 : 1;
            }
        }
        else
        {
            getMove_straight(trog);
        }
    }
    else
    {
        getMove_straight(trog);
    }
}

void getMove_run(Troggle *trog)
{
    int dx, dy;
    Player *m = game->getNearestMuncher(trog->x, trog->y);

    if( m )
    {
        Point munch_pos = m->getPos();

        dx = trog->x - munch_pos.x;
        dy = trog->y - munch_pos.y;

        if(dx < 0)
            dx *= -1;
        if(dy < 0)
            dy *= -1;

        if(m->exist() && dx <= 2 && dy <= 2)
        {
            if(dx > dy || (dx == dy && rand()%2))
            {
                trog->x -= (trog->x - munch_pos.x > 0)? -1 : 1;
            }
            else
            {
                trog->y -= (trog->y - munch_pos.y > 0)? -1 : 1;
            }
        }
        else
        {
            getMove_random(trog);
        }
    }
    else
    {
        getMove_random(trog);
    }
}

void onMove_create(Troggle *trog)
{
    game->setNum( trog->x, trog->y, game->randomNumber() );
}

void onStop_munch(Troggle *trog)
{
    game->playerMunch( trog, trog->x, trog->y, SDL_GetTicks(), true );
}
