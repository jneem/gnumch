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
#include "TextField.h"
#include "Menu.h"

TextField::TextField(int size)
{
    int dummy, advance;
    TTF_Font *font = Menu::getButtonFont();

    TTF_GlyphMetrics(font, 'm', &dummy, &dummy, &dummy, &dummy, &advance);
    text.push_back(' ');
    setWidth(advance * size);
    cursor = 0;
    maxsize = size;
    rawkeyevents = true;
    allow_key_focus = true;
}

TextField::~TextField()
{
}

void TextField::handleClick(const SDL_Event *event)
{
    /* FIXME: position the cursor in the correct place within the text stream
     */
}

bool TextField::handleRawKey(const SDL_Event *event)
{
    SDLKey sym = event->key.keysym.sym;

    if (sym == SDLK_UP || sym == SDLK_DOWN || sym == SDLK_TAB) {
        return false;
    }
    if (sym == SDLK_LEFT && cursor>0) {
        cursor--;
    } else if (sym == SDLK_RIGHT && cursor<(int)text.size()-1) {
        cursor++;
    } else if (sym == SDLK_BACKSPACE && cursor>0) {
        cursor--;
        text.erase(cursor, 1);
    } else if(!isalnum(sym) && !ispunct(sym)) {
        return true;
    } else if(cursor == maxsize) {
        return true;
    } else if(!event->key.keysym.unicode) {
        /* only allow ASCII characters if there is no UNICODE */
        if (    (sym < '0' || sym > '9') &&
                (sym < 'a' || sym > 'z') &&
                (sym < 'A' || sym > 'Z') ) {
            return true;
        }
        text.insert(cursor, 1, (char)sym);
        cursor++;
    } else {
        char utf8[MB_CUR_MAX];
        int chsize = wctomb(utf8, event->key.keysym.unicode);
        text.insert(cursor, utf8, chsize);
        cursor++;
    }
    setChanged(1);
    return true;
}

/* don't allow resizing to less than advance*maxsize */
void TextField::getMinSize(int *w, int *h)
{
    int tmp_w, tmp_h;
    int advance, d;

    TTF_GlyphMetrics(Menu::getButtonFont(), 'm', &d, &d, &d, &d, &advance);
    Label::getMinSize(&tmp_w, &tmp_h);

    *w = max(tmp_w, advance*maxsize);
    *h = tmp_h;
}

void TextField::draw(bool update)
{
    int d, adv;
    TTF_GlyphMetrics(Menu::getButtonFont(), '_', &d, &d, &d, &d, &adv);

    getSize(&pos.w, &pos.h);
    if (!changed && drawn_pos == pos) return;

    drawn_pos.w += adv; // trick Label into clearing up the cursor
    Label::draw(update);

    Rectangle r;
    getCursorPos(&r);

    if (keyboard_focused) {
        Menu::drawString(string("_"), r, 0);
        if (update) {
            Menu::update(r);
        }
    }
}

void TextField::getCursorPos(Rectangle *r)
{
    string partial(text, 0, cursor);
    int w, h;
    int adv, d, font_h;

    font_h = TTF_FontHeight(Menu::getButtonFont());
    TTF_GlyphMetrics(Menu::getButtonFont(), '_', &d, &d, &d, &d, &adv);
    Menu::getStringSize(partial, &w, &h);
    r->x = drawn_text_pos.x + w;
    r->y = drawn_text_pos.y;
    r->h = font_h;
    r->w = adv;
}
