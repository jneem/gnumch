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
#include "Button.h"
#include <iostream>
#include "Menu.h"
#include <FileSys.h>

extern FileSys *fs;

Clickable::Clickable()
{
    setChanged(1);
    clicked = NULL;
    clicked_data = NULL;
}

void Clickable::setCallback(Callback fn, void *data)
{
    clicked = fn;
    clicked_data = data;
}

void Clickable::handleClick(const MouseEvent_t &event) {
    if (event.type == CLICK && event.button == SDL_BUTTON_LEFT) {
        Menu::playClick();
        if(clicked) (*clicked)(this, clicked_data);
    }
}

void Clickable::handleKey(const SDL_Event *event) {
    if (event->key.keysym.sym == SDLK_RETURN
            || event->key.keysym.sym == ' ') {
        Menu::playClick();
        if (clicked) (*clicked)(this, clicked_data);
    }
}

void Clickable::focus(bool update)
{
    if(!focused) {
        focused = 1;
        if(update) {
            changed = 1;
            draw(1);
        } else {
            setChanged(1);
        }
    }
}

void Clickable::keyboardFocus(bool update)
{
    if(!keyboard_focused) {
        keyboard_focused = 1;
        if(update) {
            changed = 1;
            draw(1);
        } else {
            setChanged(1);
        }
    }
}

void Clickable::unfocus(bool update)
{
    if(focused) {
        focused = 0;
        if(update) {
            changed = 1;
            draw(1);
        } else {
            setChanged(1);
        }
    }
}

void Clickable::keyboardUnfocus(bool update)
{
    if(keyboard_focused) {
        keyboard_focused = 0;
        if(update) {
            changed = 1;
            draw(1);
        } else {
            setChanged(1);
        }
    }
}

TextButton::TextButton(const string &str)
{
    if(!str.empty()) {
        text = str.c_str();
    }
    setChanged(1);
}

TextButton::~TextButton()
{
}

void TextButton::draw(bool update)
{
    Uint32 color = Menu::getButtonColor(focused, keyboard_focused, highlight);

    Rectangle textdest;
    getSize(&pos.w, &pos.h);

    if (!changed && drawn_pos == pos)
        return;

    Menu::getStringSize(text, &textdest.w, &textdest.h);

    /* position the text */
    textdest.y = pos.y + (pos.h - textdest.h)/2;
    switch(text_align) {
        case TEXT_LEFT:
            textdest.x = pos.x + textdest.h/2;
            break;
        case TEXT_CENTER:
            textdest.x = pos.x + (pos.w - textdest.w)/2;
            break;
        case TEXT_RIGHT:
            textdest.x = pos.x + pos.y - textdest.w - textdest.h/2;
            break;
    }

    Menu::drawButton(pos, color, 0);
    Menu::drawString(text, textdest, 0);
    if (update) {
        Menu::update(pos);
    }

    drawn_pos = pos;
    setChanged(0);
}

void TextButton::getMinSize(int *w_, int *h_)
{
    int w_text, h_text;
    Menu::getStringSize(text, &w_text, &h_text);
    *w_ = max(h_text, w_text) + h_text;
    *h_ = h_text * 2;
}

void TextButton::setText(const string &new_text)
{
    TextWidget::setText(new_text);
    setChanged(1);
}

PicButton::PicButton(const char *unf, const char *f, const char *k, const char *h)
{
    pic = fs->openPic(unf, -1, -1);
    fpic = f ? fs->openPic(f, -1, -1) : NULL;
    kpic = k ? fs->openPic(k, -1, -1) : NULL;
    hpic = h ? fs->openPic(h, -1, -1) : NULL;
}

PicButton::~PicButton()
{
    SDL_FreeSurface(pic);
    if (fpic) SDL_FreeSurface(fpic);
    if (kpic) SDL_FreeSurface(kpic);
    if (hpic) SDL_FreeSurface(hpic);
}

void PicButton::draw(bool update)
{
    if (!changed) return;
    getSize (&pos.w, &pos.h);

    SDL_Surface *pic = this->pic;

    if ( fpic && (focused || (keyboard_focused && !kpic)) )
                                        pic = this->fpic;
    else if (keyboard_focused && kpic)  pic = this->kpic;
    else if (highlight && hpic)         pic = this->hpic;

    if(pos.w != pic->w || pos.h != pic->h) {
        SDL_Surface *tmp;
        double zoomx = (double)pos.w/pic->w;
        double zoomy = (double)pos.h/pic->h;
        tmp = zoomSurface(pic, zoomx, zoomy, 1);
        Menu::drawPic(tmp, pos, update);
        SDL_FreeSurface(tmp);
    } else {
        Menu::drawPic(pic, pos);
    }
    if (update) {
        Menu::update(pos);
    }
    setChanged(0);
    drawn_pos = pos;
}

void PicButton::getMinSize(int *w_, int *h_)
{
    *w_ = pic->w;
    *h_ = pic->h;
}

