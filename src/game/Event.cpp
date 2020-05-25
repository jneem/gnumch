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

#include <Event.h>
#include <FileSys.h>
#include <ConfigFile.h>

extern FileSys *fs;

SDLKey default_keys1[KEY_NUM] = {SDLK_w, SDLK_s, SDLK_a, SDLK_d,
                                 SDLK_SPACE, SDLK_z, SDLK_ESCAPE};
SDLKey default_keys2[KEY_NUM] = {SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
                                 SDLK_RCTRL, SDLK_RETURN, SDLK_END};

Event::Event(const char *ev_string)
{
    str = ev_string;
    parseStr();
}

Event::Event(const SDL_Event& ev)
{
    ev_data = ev;
}

Event::Event(SDLKey key)
{
    ev_data.type = SDL_KEYDOWN;
    ev_data.key.keysym.sym = key;
}

Event::Event()
{
    ev_data.type = SDL_USEREVENT;
}

Event::Event(const Event &e)
{
    if (e.toString()) {
        str = e.toString();
        parseStr();
    } else {
        ev_data.type = SDL_USEREVENT;
    }
}

Event::~Event()
{
}

const char *Event::toString() const
{
    if (ev_data.type == SDL_USEREVENT) {
        return NULL;
    } else if (str.size() == 0) {
        makeStr();
    }
    return str.c_str();
}

const char *Event::toLongString() const
{
    if (ev_data.type == SDL_USEREVENT) {
        return _("None");
    } else if (long_str.size() == 0) {
        makeLongStr();
    }
    return long_str.c_str();
}

bool Event::empty() const
{
    return (ev_data.type == SDL_USEREVENT);
}

void Event::parseStr()
{
    int arg1, arg2, arg3;

    if (str.size() != 6) {
        printWarning("wrong-sized event specifier \"%s\"\n", str.c_str());
        return;
    }

    if (!str.compare(0, 3, "kbd")) {
        ev_data.type = SDL_KEYDOWN;
        sscanf(str.c_str(), "kbd%03d", &arg1);
        ev_data.key.keysym.sym = (SDLKey)arg1;
    } else if (!str.compare(0, 2, "ja")) {
        char dir[3];
        ev_data.type = SDL_JOYAXISMOTION;
        sscanf(str.c_str(), "ja%1d%1d%2s", &arg1,
                                           &arg2,
                                           dir);
        ev_data.jaxis.which = arg1;
        ev_data.jaxis.axis  = arg2;
        ev_data.jaxis.value = strcmp(dir, "PO") ? -1 : 1;
    } else if (!str.compare(0, 2, "jh")) {
        ev_data.type = SDL_JOYHATMOTION;
        sscanf(str.c_str(), "jh%1d%1d%02d", &arg1,
                                            &arg2,
                                            &arg3);
        ev_data.jhat.which = arg1;
        ev_data.jhat.hat   = arg2;
        ev_data.jhat.value = arg3;
    } else if (!str.compare(0, 2, "jb")) {
        ev_data.type = SDL_JOYBUTTONDOWN;
        sscanf(str.c_str(), "jb%1d%03d", &arg1,
                                         &arg2);
        ev_data.jbutton.which  = arg1;
        ev_data.jbutton.button = arg2;
    } else {
        printWarning("unparseable event specifier \"%s\"\n", str.c_str());
    }
}

void Event::makeStr() const
{
    char tmp[7];

    switch (ev_data.type) {
        case SDL_KEYDOWN:
            snprintf(tmp, 7, "kbd%03d", ev_data.key.keysym.sym);
            break;
        case SDL_JOYAXISMOTION:
            snprintf(tmp, 7, "ja%1d%1d%2s", ev_data.jaxis.which,
                                            ev_data.jaxis.axis,
                                            ev_data.jaxis.value < 0 ? "NE" : "PO");
            break;
        case SDL_JOYHATMOTION:
            snprintf(tmp, 7, "jh%1d%1d%02d", ev_data.jhat.which,
                                             ev_data.jhat.hat,
                                             ev_data.jhat.value);
            break;
        case SDL_JOYBUTTONDOWN:
            snprintf(tmp, 7, "jb%1d%03d", ev_data.jbutton.which,
                                          ev_data.jbutton.button);
            break;
        default:
            return;
    }
    str = tmp;
}

void Event::makeLongStr() const
{
    char tmp[20];

    switch (ev_data.type) {
        case SDL_KEYDOWN:
            long_str = _("Keyboard ");
            long_str = SDL_GetKeyName(ev_data.key.keysym.sym);
            break;
        case SDL_JOYAXISMOTION:
            snprintf(tmp, 20, _("Joy%1d axis "), ev_data.jaxis.which);
            long_str = tmp;
            if (ev_data.jaxis.axis == 0) {
                if (ev_data.jaxis.value > 0) {
                    long_str += _("right");
                } else {
                    long_str += _("left");
                }
            } else {
                if (ev_data.jaxis.value > 0) {
                    long_str += _("down");
                } else {
                    long_str += _("up");
                }
            }
            break;
        case SDL_JOYHATMOTION:
            snprintf(tmp, 20, _("Joy%1d hat%1d "), ev_data.jhat.which,
                                                  ev_data.jhat.hat);
            long_str = tmp;
            if (ev_data.jhat.value & SDL_HAT_UP) {
                long_str += _("up");
            } else if (ev_data.jhat.value & SDL_HAT_DOWN) {
                long_str += _("down");
            } else if (ev_data.jhat.value & SDL_HAT_LEFT) {
                long_str += _("left");
            } else if (ev_data.jhat.value & SDL_HAT_RIGHT) {
                long_str += _("right");
            } else {
                long_str += _("error");
            }
            break;
        case SDL_JOYBUTTONDOWN:
            snprintf(tmp, 20, _("Joy%1d button %3d"), ev_data.jbutton.which,
                                                      ev_data.jbutton.button);
            long_str = tmp;
            break;
        default:
            long_str = "None";
            return;
    }
}

bool Event::pollEvent()
{
    bool got_event = false;

    str.clear();
    long_str.clear();

    do {
        got_event = SDL_PollEvent(&ev_data);
    } while (got_event && !(ev_data.type == SDL_KEYDOWN
                        ||  ev_data.type == SDL_JOYAXISMOTION
                        ||  ev_data.type == SDL_JOYHATMOTION
                        ||  ev_data.type == SDL_JOYBUTTONDOWN));
    if (got_event) {
        printMsg(1, "got event \"%s\"\n", toLongString());
    }
    return got_event;
}

void Event::waitEvent()
{
    str.clear();
    long_str.clear();

    do {
        SDL_WaitEvent(&ev_data);
    } while (    !(ev_data.type == SDL_KEYDOWN
                || ev_data.type == SDL_JOYAXISMOTION
                || ev_data.type == SDL_JOYHATMOTION
                || ev_data.type == SDL_JOYBUTTONDOWN));
    printMsg(1, "got event \"%s\"\n", toLongString());
}

KeyBindings::KeyBindings(const char *section)
{
    ConfigFile *conf = fs->openConfig("gnumch.cfg");
    const char *keyspec;
    char key[9];

    this->section = section;

    for (int i=0; i<KEY_NUM; i++) {
        primary[i] = Event();
        secondary[i] = Event();
        name[i] = "";
    }

    for (int i=0; i<KEY_NUM; i++) {
        sprintf(key, "prikey%02d", i);
        keyspec = conf->readString(section, key, NULL);
        if (keyspec) {
            Event tmp(keyspec);
            addEvent((enum Key)i, tmp);
        }
        sprintf(key, "seckey%02d", i);
        keyspec = conf->readString(section, key, NULL);
        if (keyspec) {
            Event tmp(keyspec);
            addEvent((enum Key)i, tmp);
        }
    }
    delete conf;
}

KeyBindings::KeyBindings()
{
    for (int i=0; i<KEY_NUM; i++) {
        primary[i] = Event();
        secondary[i] = Event();
        generateName(i);
    }
}

KeyBindings::KeyBindings(const KeyBindings &bind)
{
    section = bind.section;
    for (int i=0; i<KEY_NUM; i++) {
        primary[i] = Event(bind.getPrimaryEvent((enum Key)i));
        secondary[i] = Event(bind.getSecondaryEvent((enum Key)i));
        generateName(i);
    }
}

KeyBindings::~KeyBindings()
{
}

enum Key KeyBindings::getKey(const Event &e) const
{
    for (int i=0; i<KEY_NUM; i++) {
        if ((!primary[i].empty()
                    && !strcmp(primary[i].toString(), e.toString()))
                || (!secondary[i].empty()
                    && !strcmp(secondary[i].toString(), e.toString()))) {
            return (enum Key)i;
        }
    }
    return KEY_NUM;
}

bool KeyBindings::hasEvent(enum Key key) const
{
    return !(primary[key].empty() && secondary[key].empty());
}

const Event &KeyBindings::getPrimaryEvent(enum Key key) const
{
    return primary[key];
}

const Event &KeyBindings::getSecondaryEvent(enum Key key) const
{
    return secondary[key];
}

const char *KeyBindings::getName(enum Key key) const
{
    if (name[key].empty()) {
        return _("None");
    }
    return name[key].c_str();
}

void KeyBindings::addEvent(enum Key key, const Event &e)
{
    if (primary[key].empty()) {
        primary[key] = e;
    } else {
        if (!secondary[key].empty()) {
            deleteEvent(e);
        }
        secondary[key] = e;
    }
    generateName(key);
}

void KeyBindings::deleteEvent(const Event &e)
{
    const char *str = e.toString();

    if (!str) {
        printWarning("tried to delete empty event\n");
        return;
    }

    for (int i=0; i<KEY_NUM; i++) {
        if (!secondary[i].empty() && !strcmp(secondary[i].toString(), str)) {
            secondary[i] = Event();
            generateName(i);
        }
        if (!primary[i].empty() && !strcmp(primary[i].toString(), str)) {
            if (secondary[i].empty()) {
                primary[i] = Event();
            } else {
                primary[i] = secondary[i];
                secondary[i] = Event();
            }
            generateName(i);
        }
    }
}

void KeyBindings::generateName(int i)
{
    assert( i >= 0 && i < KEY_NUM );

    name[i].clear();
    if (primary[i].empty()) {
        name[i] = _("None");
    } else {
        name[i] = primary[i].toLongString();
    }

    if (!secondary[i].empty()) {
        name[i] += _(" or ");
        name[i] += secondary[i].toLongString();
    }
}

void KeyBindings::writeSettings() const
{
    if (this->section.empty()) {
        return;
    }

    ConfigFile *conf = fs->openConfig("gnumch.cfg", true);
    char key[9];

    for (int i=0; i<KEY_NUM; i++) {
        if (!primary[i].empty()) {
            sprintf(key, "prikey%02d", i);
            conf->writeString(this->section.c_str(), key, primary[i].toString());
        }
        if (!secondary[i].empty()) {
            sprintf(key, "seckey%02d", i);
            conf->writeString(this->section.c_str(), key, secondary[i].toString());
        }
    }
    delete conf;
}
