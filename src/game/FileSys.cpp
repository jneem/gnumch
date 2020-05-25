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
#include <FileSys.h>
#include <Game.h>
#include <ConfigFile.h>

extern Game *game;

SDL_mutex *FileSys::fs_mutex;

FileSys::FileSys()
{
    char *home = getenv("HOME");

    char main[strlen(DATADIR) + strlen(PACKAGE) + 3];
    sprintf(main, "%s/%s/", DATADIR, PACKAGE);

    if(home) {
        char backup[strlen(home) + strlen(PACKAGE) + 4];
        sprintf(backup, "%s/.%s/", home, PACKAGE);
        base = new Directory(backup, main);
    } else {
        base = new Directory(main, NULL);
    }

    /* set up the directory structure */
    anim        = new Directory(base, "animation"   );
    anim_pic    = new Directory(    anim, "pics"    );
    anim_sound  = new Directory(    anim, "sounds"  );
    font        = new Directory(base, "fonts"       );
    pic         = new Directory(base, "pics"        );
    sound       = new Directory(base, "sounds"      );

    if (!fs_mutex) fs_mutex = SDL_CreateMutex();
}

FileSys::~FileSys()
{
    delete base;
    delete anim;
    delete anim_pic;
    delete anim_sound;
    delete font;
    delete pic;
}

TTF_Font *FileSys::openFont(const char *name, int size)
{
    TTF_Font *ret;
    string filename;

    SDL_mutexP(fs_mutex);
    getFile(font, filename, name);
    SDL_mutexV(fs_mutex);

    ret = TTF_OpenFont(filename.c_str(), size);
    if(!ret) {
        printError("Couldn't open font: %s\n", TTF_GetError());
    }
    return ret;
}

SDL_Surface *FileSys::openPic(const char *name, int w, int h)
{
    if(!name) return NULL;
    string filename;

    SDL_mutexP(fs_mutex);
    getFile(pic, filename, name);
    SDL_Surface *ret = openAnimPic(filename, w, h);
    SDL_mutexV(fs_mutex);

    return ret;
}

SDL_Surface *FileSys::openAnimPic(const char *name, int w, int h)
{
    if (!name) return NULL;
    string filename;

    SDL_mutexP(fs_mutex);
    getFile(anim_pic, filename, name);
    SDL_Surface *ret = openAnimPic(filename, w, h);
    SDL_mutexV(fs_mutex);

    return ret;
}

Mix_Chunk *FileSys::openAnimSound(const char *name)
{
    if (!name) return NULL;
    string filename;

    SDL_mutexP(fs_mutex);
    getFile(anim_sound, filename, name);
    Mix_Chunk *ret = Mix_LoadWAV(filename.c_str());
    SDL_mutexV(fs_mutex);

    return ret;
}

Mix_Chunk *FileSys::openSound(const char *name)
{
    if (!name) return NULL;
    string filename;

    SDL_mutexP(fs_mutex);
    getFile(sound, filename, name);
    Mix_Chunk *ret = Mix_LoadWAV(filename.c_str());
    SDL_mutexV(fs_mutex);

    if (!ret) printWarning("couldn't open sound %s: %s\n", name, Mix_GetError());

    return ret;
}

void FileSys::getFullPath(string *full, const char *name, int dir) throw (int)
{
    Directory *d;

    switch(dir) {
        case DIR_MAIN:
            d = base;
            break;
        case DIR_ANIM:
            d = anim;
            break;
        case DIR_ANIM_PIC:
            d = anim_pic;
            break;
        case DIR_ANIM_SOUND:
            d = anim_sound;
            break;
        case DIR_FONT:
            d = font;
            break;
        default:
            printWarning("unrecognised directory passed to FileSys\n");
            throw ENOENT;
    }
    getFile(d, *full, name);
}

FILE *FileSys::openCfg(const char *name, bool write)
{
    string filename;
    SDL_mutexP(fs_mutex);
    getFile(base, filename, name);
    SDL_mutexV(fs_mutex);

    FILE *ret = fopen(filename.c_str(), "r");
    if(!ret) printError("couldn't open file %s\n", name);

    return ret;
}

ConfigFile *FileSys::openConfig(const char *name, bool write)
{
    string fullname;

    SDL_mutexP(fs_mutex);
    try {
        getFile(base, fullname, name);
    } catch (int e) {
        if (e == ENOENT) {
            printWarning("config file %s doesn't exist, creating it\n", name);
            write = true;
        } else {
            printError("couldn't open config file %s: %s\n", name, strerror(e));
        }
    }
    SDL_mutexV(fs_mutex);
    return new ConfigFile(fullname, write);
}

void FileSys::readScores(vector<string> &names, vector<int> &scores)
{
    string filename;
    names.clear();
    scores.clear();
    SDL_mutexP(fs_mutex);
    try {
        getFile(base, filename, "scores.txt");
        ifstream fin(filename.c_str());
        string tmp_s;
        int tmp_i;

        while( (fin >> tmp_s) && fin.good() ) {
            names.push_back(tmp_s);
            fin >> tmp_i;
            scores.push_back(tmp_i);
        }
    } catch(int i) {
        printWarning("high scores file doesn't exist\n");
    }
    SDL_mutexV(fs_mutex);
}

bool FileSys::checkScore(int score)
{
    vector<string> names;
    vector<int> scores;

    readScores(names, scores);
    return (scores.size() < 10) || (score > scores.back());
}

void FileSys::saveScore(const char *name, int score)
{
    vector<string> names;
    vector<int> scores;
    vector<string>::iterator s;
    vector<int>::iterator i;

    readScores(names, scores);
    for(i=scores.begin(), s=names.begin(); i<scores.end() && *i>=score; i++, s++)
        ;

    names.insert(s, name);
    scores.insert(i, score);
    if(names.size() > 10) {
        names.pop_back();
        scores.pop_back();
    }

    string filename;
    ofstream fout;

    SDL_mutexP(fs_mutex);
    try {
        getCfg(filename, "scores.txt");
    } catch(int i) {
        if(i == ENOENT) {
            cout << "creating config file\n";
        } else {
            printError("unexpected exception %d\n", i);
        }
    }
    SDL_mutexV(fs_mutex);
    fout.open(filename.c_str(), ios::trunc);
    for(i=scores.begin(), s=names.begin(); i<scores.end(); i++, s++) {
        fout << *s << " " << *i << endl;
    }
    fout.close();
}

void FileSys::getCfg(string &ret, const string &name) throw (int)
{
    getFile(base, ret, name);
}

/*_______________________________utility functions____________________________*/
extern int errno;
SDL_Surface *FileSys::openAnimPic(const string &name, int width, int height)
{
    SDL_Surface *ret, *tmp;
    double zoomx, zoomy;

    tmp = IMG_Load(name.c_str());
    if(!tmp) {
      printError("couldn't open image file %s: %s\n", name.c_str(),
		 IMG_GetError ());
    }
    printMsg(1, "opened image file %s\n", name.c_str());

    zoomx = (double)width/tmp->w;
    zoomy = (double)height/tmp->h;

    if(width == -1 && height == -1) { 
        ret = tmp;
    } else {
        if(width == -1)         zoomx = zoomy;
        else if(height == -1)   zoomy = zoomx;
        ret = zoomSurface(tmp, zoomx, zoomy, 1);
        SDL_FreeSurface(tmp);
    }

    return ret;
}

vector<string> *FileSys::scanDir(Directory *dir, string pattern)
{
    vector<string> *ret = new vector<string>();
    string file;

    dir->rewind();

    while( !(file = dir->read()).empty() ) {
        if(pattern.empty() || file.find(pattern, 0) != string::npos) {
            ret->push_back(file);
        }
    }
    return ret;
}

void FileSys::getFile(Directory *dir, string &full, const string &name)
                      throw (int)
{
    string ret, dirname;
    dir->rewind();
    while( !(ret = dir->read(&dirname)).empty() ) {
        if(ret == name) {
            full = dirname + ret;
            return;
        }
    }
    full = dir->name() + name;
    printWarning("file not found: %s\n", full.c_str());
    throw (int)ENOENT;
}

/*________________________________Directory___________________________________*/

Directory::Directory(const char *one, const char *two)
{
    printMsg(2, "new directory: %s,%s\n", one, two);
    main = opendir(one);
    if(!main) {
        mkdir(one, 0755);
        main = opendir(one);
    }
    mainname = strdup(one);
    if(two) {
        backup = opendir(two);
        backupname = strdup(two);
    } else {
        backup = NULL;
        backupname = NULL;
    }

    if(!main)             printError("couldn't open directory %s\n", one);
    if(two && (!backup) ) printError("couldn't open directory %s\n", two);
}

Directory::Directory(Directory *parent, const char *subdir)
{
    printMsg(2, "new subdir: {%s,%s}/%s\n", parent->mainname,
                                            parent->backupname, subdir);
    mainname = (char*)malloc(strlen(parent->mainname) + strlen(subdir) + 2);
    sprintf(mainname, "%s%s/", parent->mainname, subdir);
    if(parent->backup) {
        backupname = (char*)malloc(strlen(parent->backupname)+strlen(subdir)+2);
        sprintf(backupname, "%s%s/", parent->backupname, subdir);
        backup = opendir(backupname);
    } else {
        backupname = NULL;
        backup = NULL;
    }
    main = opendir(mainname);
    if(!main && backup) {
        printMsg(1, "couldn't open directory %s\n", mainname);
        free(mainname);
        main = backup;
        mainname = backupname;
        backup = NULL;
        backupname = NULL;
    } else if(main && backupname && !backup) {
        printWarning("couldn't open directory %s\n", backupname);
        free(backupname);
        backupname = NULL;
    } else if(!main && !backup) {
        printError("couldn't open required directory %s\n", mainname);
        free(mainname);
        if(backupname) free(backupname);
    }
}

Directory::~Directory()
{
    free(mainname);
    free(backupname);
    closedir(main);
    closedir(backup);
}

void Directory::rewind()
{
    rewinddir(main);
    if(backup) rewinddir(backup);
}

string Directory::read()
{
    struct dirent *ent;

    ent = readdir(main);
    if(ent) return (string)mainname + ent->d_name;

    if(backup) {
        ent = readdir(backup);
        if(ent) return (string)backupname + ent->d_name;
    }

    return "";
}

string Directory::read(string *dirname)
{
    struct dirent *ent;

    ent = readdir(main);
    if(ent) *dirname = mainname;

    if(!ent && backup) {
        ent = readdir(backup);
        if(ent) *dirname = backupname;
    }

    if(ent) return ent->d_name;
    return "";
}

string Directory::readShort()
{
    struct dirent *ent;

    ent = readdir(main);

    if(!ent && backup) {
        ent = readdir(backup);
    }

    if(ent) return ent->d_name;
    return "";
}

const char *Directory::name()
{
    return mainname;
}
