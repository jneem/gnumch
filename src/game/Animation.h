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
#ifndef ANIMATION_H
#define ANIMATION_H

#include <Gnumch.h>

typedef enum {
    ANIM_NORMAL,
    ANIM_WALKING,
    ANIM_EATING,
    ANIM_APPEARING,
    ANIM_DISAPPEARING,
    ANIM_NUM
} AnimationType;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NUM
} AnimationDirection;

struct Picture {
    union {
        SDL_Surface *pic;
        struct { /* link info */
            int type;
            int dir;
            int frame;
        };
    };
    bool linked;
};

typedef struct Picture Picture;

class Animation {
    public:
        SDL_Surface     *getFrame   (int, int, int);
        Mix_Chunk       *getSound   (int);
        int             getLoops    (int);
        int             milliseconds(int, int);
        int             frame       (int, int);
        void            load        ();
        void            reloadPics  ();
        void            reloadSounds();
        void            ready       ();

        Animation(const char*);
        ~Animation();

    protected:
        char *basename;
        int *frames;
        int *mpf;
        Picture **pic;
        string sound_file[ANIM_NUM];
        Mix_Chunk *sound[ANIM_NUM];
        int sound_loops[ANIM_NUM];

        enum {
            UNLOADED,
            LOADING,
            LOADED
        };
        int loadstate;
        SDL_Thread *loadthread;
        static SDL_mutex *loadmutex;

        Picture &p(int, int, int);

        void startLoading();
        void finishLoading();
        void unloadPics();
        void unloadSounds();
        void loadConfig();
        void parseLink(Picture*, const char *text);

        static int loadThread(Animation *a);
};

class AnimationState {
    public:
        AnimationState(Animation*);
        SDL_Surface     *getFrame   ();
        int             nextFrame   ();
        bool            finished    () const;     /* returns whether the current animation
                                                     has finished running. */
        void            setState    (AnimationType);
        void            stopSound   ();
        void            setDir      (AnimationDirection);
        void            setAnim     (Animation*);
        AnimationType   getState    ();
        AnimationDirection getDir   ();

    protected:
        Animation *anim;
        AnimationType type;
        AnimationDirection dir;
        int frame;
        int last;
        int sound_channel;
};

#endif
