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
#include "Menu.h"
#include "Label.h"
#include <FileSys.h>

extern FileSys *fs;

Label::Label(const string &str)
{
    if(!str.empty()) {
        text = str.c_str();
    }
    allow_key_focus = false;
}

void Label::draw(bool update)
{
    Rectangle textdest;
    Rectangle refresh;
    Uint32 color = Menu::getButtonColor(0, 0, highlight);

    if (!changed) {
        return;
    }

    getSize(&pos.w, &pos.h);

    Menu::getStringSize(text, &textdest.w, &textdest.h);

    /* draw the text */
    textdest.y = pos.y + (pos.h - textdest.h)/2;
    switch(text_align) {
        case TEXT_LEFT:
            textdest.x = pos.x + textdest.h/2;
            break;
        case TEXT_CENTER:
            textdest.x = pos.x + (pos.w - textdest.w)/2;
            break;
        case TEXT_RIGHT:
            textdest.x = pos.x + pos.w - textdest.w - textdest.h/2;
            break;
    }
    if (pos == drawn_pos && drawn_text_pos.x != -1) {
        Menu::drawBox(drawn_text_pos, color);
        refresh = drawn_text_pos + textdest;
    } else {
        Menu::drawButton(pos, Menu::getWhite(), 0);
        refresh = pos;
    }
    Menu::drawString(text, textdest, 0);

    if (update) {
        Menu::update(refresh);
    }

    drawn_pos = pos;
    drawn_text_pos = textdest;
    setChanged(0);
}

void Label::getMinSize(int *w_, int *h_)
{
    int w_text, h_text;
    Menu::getStringSize(text, &w_text, &h_text);
    *w_ = max(w_text, h_text) + h_text;
    *h_ = h_text * 2;
}

void Label::setText(const string &new_text)
{
    TextWidget::setText(new_text);
    setChanged(1);
}

PicLabel::PicLabel(string filename, int w_, int h_):
    pic(NULL),
    private_surface(false)
{
    allow_key_focus = false;
    setPic(filename, w_, h_);
}

PicLabel::PicLabel(SDL_Surface *s):
    pic(NULL),
    private_surface(false)
{
    allow_key_focus = false;
    setPic(s);
}

PicLabel::PicLabel():
    pic(NULL),
    private_surface(false)
{
    allow_key_focus = false;
}

PicLabel::~PicLabel()
{
    if(pic && private_surface) SDL_FreeSurface(pic);
}

void PicLabel::setPic(string s, int w_, int h_)
{
    if (pic && private_surface) SDL_FreeSurface(pic);
    pic = fs->openPic(s.c_str(), w_, h_);
    private_surface = true;
}

void PicLabel::setPic(SDL_Surface *s)
{
    if (pic && private_surface) SDL_FreeSurface(pic);
    pic = s;
    private_surface = false;
}

void PicLabel::draw(bool update)
{
    Rectangle dest (pos.x + (pos.w-pic->w)/2,
                    pos.y + (pos.h-pic->h)/2,
                    pic->w, pic->h);
    Menu::drawPic(pic, dest, update);
}

void PicLabel::getMinSize(int *w_, int *h_)
{
    *w_ = pic->w + 2;
    *h_ = pic->h + 2;
}

