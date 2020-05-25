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
#ifndef FILESYS_H
#define FILESYS_H

#include <Gnumch.h>

class Animation;
typedef struct Picture Picture;
class ConfigFile;

class Directory {
    public:
        Directory(const char*, const char*);
        Directory(Directory*, const char*);
        ~Directory();

        void rewind();
        string read();
        string read(string*);
        string readShort();
        const char *name();

    protected:
        DIR *main;
        DIR *backup;
        char *mainname;
        char *backupname;
};

class FileSys {
    public:

        enum {
            DIR_MAIN,
            DIR_ANIM,
            DIR_ANIM_PIC,
            DIR_ANIM_SOUND,
            DIR_FONT
        };

        FileSys();
        ~FileSys();
        TTF_Font *openFont(const char *name, int size);
        SDL_Surface *openPic(const char *name, int w, int h);
        SDL_Surface *openAnimPic(const char *name, int w, int h);
        Mix_Chunk *openSound(const char *name);
        Mix_Chunk *openAnimSound(const char *name);
        FILE *openCfg(const char*, bool write=0);
        void getCfg(string&, const string&) throw (int);
        ConfigFile *openConfig(const char *, bool write=0);

        void readScores(vector<string>&, vector<int>&);
        bool checkScore(int);
        void saveScore(const char *name, int);

        void getFullPath(string*, const char *name, int dir) throw (int);

    protected:
        Directory *base;
        Directory *anim;
        Directory *anim_pic;
        Directory *anim_sound;
        Directory *font;
        Directory *pic;
        Directory *sound;

        SDL_Surface *openAnimPic(const string&, int width, int height);
        vector<string> *scanDir(Directory*, string);
        void getFile(Directory*, string&, const string&) throw (int);

        static SDL_mutex *fs_mutex;
};

#endif
