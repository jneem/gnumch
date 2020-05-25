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
#include "Widget.h"
#include "Menu.h"

Widget::Widget()
{
    pos.x = pos.y = pos.w = pos.h = -1;
    drawn_pos.x = drawn_pos.y = drawn_pos.w = drawn_pos.h = -1;
    focused = 0;
    keyboard_focused = 0;
    allow_key_focus = true;
    parent = NULL;
    repeating = false;
    rawevents = false;
    changed   = true;
    handling_key_focus = false;
    highlight = false;
}

Widget::~Widget()
{
}

void Widget::cleanDraw()
{
    getSize(&pos.w, &pos.h);
    if (drawn_pos.x != -1 && drawn_pos != pos) {
        if (parent) {
            parent->drawBackground(drawn_pos);
        }
    }
}

/* no need to redraw if the following conditions are met:
 * 1) old min size <= current min size <= current size
 * 2) current min size <= old min size < current size
 * 3) current size = (-1, -1)
 */
void Widget::resize()
{
    int new_min_w = 0, new_min_h = 0;
    bool need_resize = false;

    if (!parent) return;
    if (pos.w == -1 && pos.h == -1) return;

    getMinSize(&new_min_w, &new_min_h);
    if (pos.w != -1) {
        if ( new_min_w > pos.w
                || (new_min_w < old_min_w && old_min_w == pos.w) ) {
            need_resize = true;
        }
    }
    if (pos.h != -1) {
        if ( new_min_h > pos.h
                || (new_min_h < old_min_h && old_min_h == pos.h) ) {
            need_resize = true;
        }
    }
    if (need_resize) {
        setSize(-1, -1);
        parent->resize();
    }
}

void Widget::saveMinSize()
{
    if (pos.w == -1 && pos.h == -1) return;
    getMinSize(&old_min_w, &old_min_h);
}

void Widget::getSize(int *w_, int *h_)
{
    /* just in case they pass us &w and &h */
    int tmp_w, tmp_h;

    if (pos.w == -1 || pos.h == -1) {
        getMinSize(&tmp_w, &tmp_h);
    }

    /* override the min values if either width or height has been set */
    if (pos.w != -1) tmp_w = pos.w;
    if (pos.h != -1) tmp_h = pos.h;
    *w_ = tmp_w;
    *h_ = tmp_h;
}

void Widget::getPosition(Rectangle *r)
{
    *r = pos;
}

void Widget::setPosition(const Rectangle &new_pos)
{
    pos = new_pos;
}

void Widget::setX(int x) {pos.x = x;}
void Widget::setY(int y) {pos.y = y;}

/* this looks pretty stupid, but it allows children to effectively override
 * this by just overriding setWidth() and setHeight()
 */
void Widget::setSize(int w, int h)
{
    setWidth(w);
    setHeight(h);
}

void Widget::setWidth(int w)
{
    pos.w = w;
}

void Widget::setHeight(int h)
{
    pos.h = h;
}

void Widget::focus(bool update)
{
    focused = 1;
}

void Widget::keyboardFocus(bool update)
{
    keyboard_focused = 1;
    setChanged(1);
}

void Widget::unfocus(bool update)
{
    focused = 0;
}

void Widget::keyboardUnfocus(bool update)
{
    keyboard_focused = 0;
    setChanged(1);
}

bool Widget::hasFocus()
{
    return focused;
}

bool Widget::hasKeyboardFocus()
{
    return keyboard_focused;
}

void Widget::handleClick(const MouseEvent_t &m)
{
    if (parent) {
        parent->handleClick(m);
    }
}

bool Widget::handleEvent(const SDL_Event *event)
{
    return false;
}

bool Widget::handleRawKey(const SDL_Event *event)
{
    return false;
}

void Widget::handleKey(const SDL_Event *event)
{
    if (parent) {
        parent->handleKey(event);
    }
}

Widget *Widget::handleKeyFocus(int direction, Widget *from)
{
    if (!from || !parent) {
        return this;
    }
    return parent->handleKeyFocus(direction, this);
}

bool Widget::contains(int x, int y)
{
    return pos.contains(x, y);
}

void Widget::setParent(Container *p)
{
    parent = p;
    setSize(-1, -1);
    setAllChanged(true);
}

void Widget::setChanged(bool c)
{
    if (c && !changed && parent) {
        parent->setChanged(c);
    }
    changed = c;
}

void Widget::setHighlight(bool h)
{
    if (highlight != h) setChanged(1);
    highlight = h;
}

void Widget::setAllChanged(bool c)
{
    changed = c;
    if (c) drawn_pos = Rectangle(-1, -1, -1, -1);
}

void Widget::erase(bool update)
{
    if (parent && drawn_pos.x != -1) {
        parent->drawBackground(drawn_pos);
        if (update) {
            Menu::update(drawn_pos);
        }
    }
}

TextWidget::TextWidget(): drawn_text_pos(-1, -1, -1, -1)
{
    text_align = TEXT_CENTER;
}

TextWidget::TextWidget(const string &str): drawn_text_pos(-1, -1, -1, -1)
{
    if(!str.empty()) {
        text = str.c_str();
    }
    text_align = TEXT_CENTER;
}

TextWidget::~TextWidget()
{
}

void TextWidget::setText(const string &new_text)
{
    if(!new_text.empty()) {
        text = new_text.c_str();
    }
}

const char *TextWidget::getText()
{
    return text.c_str();
}

void TextWidget::setTextAlign(TextAlign align)
{
    text_align = align;
}

