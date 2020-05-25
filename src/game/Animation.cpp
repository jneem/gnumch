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
#include <Animation.h>
#include <FileSys.h>
#include <Game.h>
#include <ConfigFile.h>

extern FileSys *fs;
extern Game *game;

SDL_mutex *Animation::loadmutex;

static char *anim_name[] = {"normal", "walking", "eating", "appearing", "disappearing"};
static char *direction_name[] = {"up", "down", "left", "right"};

Animation::Animation(const char *name)
{
    frames  = (int*) calloc(DIR_NUM*ANIM_NUM, sizeof(int));
    mpf     = (int*) calloc(DIR_NUM*ANIM_NUM, sizeof(int));

    basename = strdup(name);
    pic = NULL;
    loadstate = UNLOADED;

    if (!loadmutex) loadmutex = SDL_CreateMutex();

    loadConfig();
}

Animation::~Animation()
{
    if (loadstate == LOADING) {
        finishLoading();
    }
    if (loadstate == LOADED) {
        unloadPics();
        unloadSounds();
    }
    free(basename);
    free(frames);
    free(mpf);
    free(pic);
}

void Animation::reloadPics()
{
    if (loadstate == LOADING) {
        finishLoading();
    }
    if (loadstate == LOADED) {
        unloadPics();
    }

    startLoading();
}

void Animation::reloadSounds()
{
    if (loadstate == LOADING) {
        finishLoading();
    }
    if (loadstate == LOADED) {
        unloadSounds();
    }

    startLoading();
}

void Animation::load()
{
    if (loadstate == UNLOADED) {
        startLoading();
    }
}

void Animation::ready()
{
    assert(loadstate == LOADING || loadstate == LOADED);
    if (loadstate == LOADING) {
        finishLoading();
    }
}

SDL_Surface *Animation::getFrame(int i, int j, int k)
{
    if (loadstate == UNLOADED) {
        startLoading();
    }
    if (loadstate == LOADING) {
        finishLoading();
    }
    if (p(i, j, k).linked) {
        int li, lj, lk;
        li = p(i, j, k).type;
        lj = p(i, j, k).dir;
        lk = p(i, j, k).frame;
        if (p(li, lj, lk).linked) {
            printError("double dereference in animation link.\n");
        }
        return p(li, lj, lk).pic;
    }
    return p(i, j, k).pic;
}

Mix_Chunk *Animation::getSound(int i)
{
    if (loadstate == UNLOADED) startLoading();
    if (loadstate == LOADING) finishLoading();
    return sound[i];
}

int Animation::getLoops(int i)
{
    return sound_loops[i];
}

int Animation::milliseconds(int i, int j)
{
    return mpf[i*DIR_NUM + j];
}

int Animation::frame(int i, int j)
{
    return frames[i*DIR_NUM + j];
}

void Animation::startLoading()
{
    assert(loadstate == UNLOADED);

    SDL_mutexP(loadmutex);
    printMsg(1, "animation %p starting to load\n", this);
    loadthread = SDL_CreateThread( (int (*)(void*))loadThread, this);
    //loadThread(this);
    loadstate = LOADING;
}

void Animation::finishLoading()
{
    assert(loadstate == LOADING);

    printMsg(1, "animation %p finishing loading\n", this);
    SDL_WaitThread(loadthread, NULL);
    loadstate = LOADED;
}

void Animation::unloadPics()
{
    int i, j, k;

    assert (loadstate != LOADING);
    for(i=0; i<ANIM_NUM; i++) {
        for(j=0; j<DIR_NUM; j++) {
            for(k=0; k<frame(i, j); k++) {
                if(!p(i, j, k).linked) {
                    SDL_FreeSurface(p(i, j, k).pic);
                    p(i, j, k).pic = NULL;
                }
            }
        }
    }
    loadstate = UNLOADED;
}

void Animation::unloadSounds()
{
    assert (loadstate != LOADING);

    for (int i=0; i<ANIM_NUM; i++) {
        if (sound[i]) {
            Mix_FreeChunk(sound[i]);
            sound[i] = NULL;
        }
    }
    loadstate = UNLOADED;
}

int Animation::loadThread(Animation *a)
{
    string filename;
    for (int i=0; i<ANIM_NUM; i++) {
        for (int j=0; j<DIR_NUM; j++) {
            for (int k=0; k < a->frame(i, j); k++) {
                if (!a->p(i, j, k).linked && !a->p(i, j, k).pic) {
                    filename = a->basename;
                    filename += "__";
                    filename += anim_name[i];
                    filename += "_";
                    filename += direction_name[j];
                    filename += "_" + itostr(k, 2) + ".png";
                    a->p(i, j, k).pic = fs->openAnimPic(filename.c_str(),
                                                   game->getSquareWidth(),
                                                   game->getSquareHeight());
                }
            }
        }
        if (a->sound_file[i].size() && !a->sound[i]) {
            try {
                a->sound[i] = fs->openAnimSound(a->sound_file[i].c_str());
            } catch (int i) {
                perror("error opening sound file\n");
            }
            if (!a->sound[i]) {
                printWarning("couldn't open sound %s: %s\n", a->sound_file[i].c_str(), SDL_GetError());
            }
        }
    }
    printMsg(1, "animation %p, images finished loading\n", a);
    SDL_mutexV(loadmutex);
    return 0;
}

void Animation::loadConfig()
{
    string tmp = basename;
    tmp += "_anim.cfg";
    int total_pics = 0;
    int i, j;

    ConfigFile *c = fs->openConfig(tmp.c_str());
    for (i=0; i<ANIM_NUM; i++) {
        for (j=0; j<DIR_NUM; j++) {
            tmp = direction_name[j];
            tmp += "_frames";
            frames[i*DIR_NUM + j] = c->readInt(anim_name[i], tmp.c_str(), 1);
            tmp = direction_name[j];
            tmp += "_mpf";
            mpf[i*DIR_NUM + j] = c->readInt(anim_name[i], tmp.c_str(), 1000);

            total_pics += frame(i, j);
        }
        sound_file[i] = c->readString(anim_name[i], "sound", "");
        sound_loops[i] = c->readInt(anim_name[i], "sound_loops", 0);
        sound[i] = NULL;
    }

    /* allocate memory and set up the pic array */
    pic = (Picture**)calloc(1, sizeof(void*) * (ANIM_NUM*DIR_NUM)
                             + sizeof(Picture) * total_pics);
    Picture *layer2  = (Picture*)(pic + ANIM_NUM*DIR_NUM);

    for(i=0; i < ANIM_NUM*DIR_NUM; i++) {
        pic[i] = layer2;
        layer2 += frames[i];
    }

    /* work out which files are links */
    for (i=0; i<ANIM_NUM; i++) {
        for (j=0; j<DIR_NUM; j++) {
            for (int k=0; k<frame(i, j); k++) {
                tmp = "link_";
                tmp += direction_name[j];
                tmp += "_" + itostr(k);
                tmp = c->readString(anim_name[i], tmp.c_str(), "");
                if (tmp.empty()) {
                    p(i, j, k).linked = false;
                    p(i, j, k).pic = NULL;
                } else {
                    p(i, j, k).linked = true;
                    parseLink(&p(i, j, k), tmp.c_str());
                }
            }
        }
    }
    delete c;
}

void Animation::parseLink(Picture *p, const char *text)
{
    int i, j, k, ret;
    size_t n;
    char anim[12], dir[6];

    ret = sscanf(text, "%11[a-zA-Z]_%5[a-zA-Z]_%d", anim, dir, &k);
    if (ret != 3) {
        printError("malformed link spec %s\n", text);
    }

    i = j = -1; /* translate the state/direction strings to ints */
    for (n=0; n<sizeof(anim_name)/sizeof(char*); n++) {
        if(!strcmp(anim_name[n], anim)) i = n;
    }
    for (n=0; n<sizeof(direction_name)/sizeof(char*); n++) {
        if(!strcmp(direction_name[n], dir)) j=n;
    }
    if (i == -1) {
        printError("invalid animation name in link spec %s\n", text);
    }
    if (j == -1) {
        printError("invalid direction name in link spec %s\n", text);
    }
    if (k >= frame(i, j)) {
        printError("invalid frame number %d in link spec %s\n", text); 
    }
    p->type = i;
    p->dir = j;
    p->frame = k;
}

Picture &Animation::p(int i, int j, int k)
{
    return pic[i*DIR_NUM + j][k];
}

/*______________________________AnimationState________________________________*/

AnimationState::AnimationState(Animation *anim_)
{
    sound_channel = -1;
    setAnim(anim_);
}

int AnimationState::nextFrame()
{
    int now = SDL_GetTicks();

    anim->ready();
    if(now >= last + anim->milliseconds(type, dir)) {
        last    = now;
        frame   = (frame+1) % anim->frame(type, dir);
        return 1;
    }
    return 0;
}

bool AnimationState::finished() const
{
    anim->ready();
    return frame == anim->frame(type, dir)-1
        && (int)SDL_GetTicks() >= last + anim->milliseconds(type, dir);
}

AnimationType AnimationState::getState()
{
    return type;
}

AnimationDirection AnimationState::getDir()
{
    return dir;
}

SDL_Surface *AnimationState::getFrame()
{
    return anim->getFrame(type, dir, frame);
}

void AnimationState::setState(AnimationType new_type)
{
    printMsg(1, "changing state to %d\n", (int)new_type);
    type    = new_type;
    frame   = 0;
    last    = SDL_GetTicks();

    stopSound();

    Mix_Chunk *m = anim->getSound(type);
    if (m) {
        printMsg(1, "playing sound %d\n", (int)type);
        Mix_VolumeChunk(m, Menu::getFXVolume());
        sound_channel = Mix_PlayChannel(-1, m, anim->getLoops(type));
    }
}

void AnimationState::stopSound()
{
    if (sound_channel >= 0) {
        Mix_FadeOutChannel(sound_channel, 50);
        sound_channel = -1;
    }
}

void AnimationState::setDir(AnimationDirection new_dir)
{
    dir = new_dir;
}

void AnimationState::setAnim(Animation *anim_)
{
    last  = SDL_GetTicks();
    frame = 0;
    type  = ANIM_NORMAL;
    dir   = DIR_RIGHT;
    anim  = anim_;
    if (anim) anim->load();
}
