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
#include <MultiPlayerServer.h>
#include <FileSys.h>
#include <Menu.h>
#include "NetworkThread.h"
#include <Animation.h>

extern FileSys *fs;

MultiPlayerServer::MultiPlayerServer(const GameSettings &set,
                                     const Menu::VideoSettings &video,
                                     int numlocal)
{
    this->numlocal = numlocal;
    this->set   = set;
    this->video = video;

    this->mpmuncher_file = "mpmuncher.cfg";
}

MultiPlayerServer::~MultiPlayerServer()
{
}

int MultiPlayerServer::addPlayer(const string &name)
{
    /* FIXME */
    return -1;
}

void MultiPlayerServer::updateLives(Player *p, int lives)
{
    for(int i=0; i<(int)munchers.size(); i++) {
        if(munchers.at(i) == p) {
            muncher_details.at(i)->lives = lives;
            break;
        }
    }

    for(int i=0; i<(int)remote.size(); i++) {
        if(remote.at(i) == p) {
            remote_details.at(i)->lives = lives;
            break;
        }
    }
}

void MultiPlayerServer::updateScore(Player *p, int score)
{
    for(int i=0; i<(int)munchers.size(); i++) {
        if(munchers.at(i) == p) {
            muncher_details.at(i)->score = score;
            break;
        }
    }

    for(int i=0; i<(int)remote.size(); i++) {
        if(remote.at(i) == p) {
            remote_details.at(i)->score = score;
            break;
        }
    }
}

/* free all the memory used in the game. Record the high score if necessary.
 * Exit gracefully to the menu */
void MultiPlayerServer::end()
{
    lost = 1;
}

/* go on to the next level */
void MultiPlayerServer::win()
{
    printMsg(1, "you beat the level\n");
    won = 1;
}

/* run a game using the selected levelset.
 * 0) set the size parameters (eg. square_width, square_height, etc)
 * 1) open the fonts and animation pictures of the required sizes
 * 2) open the bg pic and create the board
 */
void MultiPlayerServer::run(Level *level_, SDL_Surface *screen_)
{
    unsigned int i;

    screen = screen_;
    level = level_;
    SDL_ShowCursor(0);
    SDL_FillRect(screen, NULL, Menu::getBlack());
    SDL_Flip(screen);

    video.flags  = screen->flags;
    video.bpp    = screen->format->BitsPerPixel;
    numdirty = 0;

    /* set size parameters */
    video.w         = screen->w;
    video.h         = screen->h;
    square_width    = video.w / (set.width+2);
    square_height   = video.h / (set.height+2);
    left            = square_width;
    top             = square_height;
    right           = video.w - (square_width  * set.width)  - left;
    bottom          = video.h - (square_height * set.height) - top;

    /* open fonts */
    for(i=0; i<FONT_NUM; i++) {
        font[i] = fs->openFont(fontname[i], fontsize[i]);
    }

    /* open the animations */
    FILE *cfg;
    char animname[20];
    char getmove_name[20], onmove_name[20], onstop_name[20];

    cfg = fs->openCfg(mpmuncher_file);
    while(fscanf(cfg, " %19s", animname) == 1) {
        anim.push_back( new Animation(animname) );
    }

    fclose(cfg);
    cfg = fs->openCfg(troggle_file);

    while(fscanf(cfg, " %19s", animname) == 1) {
        if(     fscanf(cfg, " %19s", getmove_name) != 1
            ||  fscanf(cfg, " %19s", onmove_name ) != 1
            ||  fscanf(cfg, " %19s", onstop_name ) != 1) {
            printError("malformed troggle config file\n");
        }
        anim.push_back(new Animation(animname));
        trogdef.push_back(new TroggleDef(anim.back(),
                                             getAction(getmove_name),
                                             getAction(onmove_name),
                                             getAction(onstop_name)));
    }
    fclose(cfg);

    /* open the bg pic and create the board */
    background = fs->openPic(background_file, video.w, video.h);
    trogwarning = fs->openPic(trogwarning_file, left, -1);

    for( int i=0; i<numlocal; i++) { // the first few anims are muncher anims
        munchers.push_back( new Muncher(anim.at(i)) );
        players.push_back(munchers.at(i));
        PlayerDetails *d = (PlayerDetails*)malloc(sizeof(PlayerDetails));
        d->id = next_id++;
        d->lives = 3;
        d->score = 0;
        muncher_details.push_back(d);
    }

    for( int i=0; i<set.trog_number; i++ ) {
        troggles.push_back( new Troggle() );
        players.push_back( troggles.at(i) );
    }
    board = new Board(set.width, set.height, players);

    /* FIXME: spawn connection-listening thread */
    lost = 0;
    while(!lost) {
        level->nextLevel();
        runLevel();
    }
    /* FIXME: stop connection-listening thread */

    /* prepare for return to menu */
    SDL_ShowCursor(1);
    SDL_SetEventFilter(menuEventFilter);

    /* free memory; levelset is freed elsewhere */
    delete board;

    for(i=0; i<trogdef.size(); i++)
        delete trogdef.at(i);
    trogdef.clear();
    for(i=0; i<anim.size(); i++)
        delete anim.at(i);
    anim.clear();
    for(i=0; i<players.size(); i++)
        delete players.at(i);
    players.clear();
    for(i=0; i<muncher_details.size(); i++)
        free(muncher_details.at(i));
    muncher_details.clear();
    for(i=0; i<remote_details.size(); i++)
        free(remote_details.at(i));
    muncher_details.clear();
    munchers.clear();
    remote.clear();
    troggles.clear();

    if(background) SDL_FreeSurface(background), background=NULL;
    if(trogwarning)SDL_FreeSurface(trogwarning), trogwarning=NULL;
    for(i=0; i<FONT_NUM; i++) {
        TTF_CloseFont(font[i]);
        font[i] = NULL;
    }

    board       = NULL;
    level       = NULL;
    background  = NULL;
    trogwarning = NULL;
}

void MultiPlayerServer::playerMove(Player *p, int old_x, int old_y,
                                              int x, int y, int time)
{
}

void MultiPlayerServer::playerStop(Player *p, int x, int y, int time)
{
}

void MultiPlayerServer::playerMunch(Player *p, int x, int y, int time,
                                    bool immune)
{
}

void MultiPlayerServer::playerSpawn(Player *p, int time)
{
}

void MultiPlayerServer::playerDie(Player *p, int time)
{
}

void MultiPlayerServer::troggleNextSpawn(Troggle *t, int time)
{
}

Player *MultiPlayerServer::getNearestMuncher(int x, int y)
{
    return NULL;
}

/*_________________________________protected__________________________________*/

/* 1) create the list of Number*s
 * 2) reset the Board
 * 3) draw the level title
 * 4) main game loop
 */
void MultiPlayerServer::runLevel()
{
    board->reset();

    FPSmanager manager;
    SDL_initFramerate(&manager);
    SDL_setFramerate(&manager, 50);
    won = 0;
    warning_on = 1;
    hideTrogWarning();

    /* draw the title */
    const char *title = level->getTitle();
    SDL_Rect rect = {0, 0, video.w, top};
    drawTextBox(title, FONT_TITLE, Menu::getWhite(), Menu::getBlack(), &rect);

    SDL_Event event;
    while(!won && !lost) {
        while( SDL_PollEvent(&event) ) {
            handleEvent( event );
        }
        board->update();
        SDL_framerateDelay(&manager);
    }
}

void MultiPlayerServer::handleEvent(const SDL_Event &event)
{
    for( int i=0; i<Muncher::KEY_NUM; i++ ) {
        if( event.key.keysym.sym == set.bindings1[i] ) {
            munchers.at(0)->handleKey((Muncher::Key)i);
            break;
        } else if( event.key.keysym.sym == set.bindings2[i]
                   && numlocal > 1 ) {
            munchers.at(1)->handleKey((Muncher::Key)i);
        }
    }
}

ServerDetails::ServerDetails(const UDPpacket *p)
{
    assert( PACKET_TYPE(p) == NetThread::PT_DETAILS );

    name.assign( (char*)&PACKET_DATA(p)[0], SERVER_NAME_LEN );
    numplayers = PACKET_DATA(p)[17];
    maxplayers = PACKET_DATA(p)[18];
    gametime = SDLNet_Read32( &PACKET_DATA(p)[19] );

    if (p->len != 23 + 23*numplayers) {
        printWarning("strangeness in details packet\n");
    }

    for (int i=0; i<numplayers; i++) {
        PlayerDetails *d = (PlayerDetails*)malloc(sizeof(PlayerDetails));
        d->id = PACKET_DATA(p)[23 + i*23];
        d->name.assign( (char*)&PACKET_DATA(p)[24 + i*23], PLAYER_NAME_LEN );
        d->score = SDLNet_Read32( &PACKET_DATA(p)[41 + i*23] );
        d->lives = PACKET_DATA(p)[45 + i*23];

        player_details.push_back(d);
    }
}

ServerDetails::~ServerDetails()
{
    for (int i=0; i<(int)player_details.size(); i++) {
        free(player_details[i]);
    }
}
