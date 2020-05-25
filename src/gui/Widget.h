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
#ifndef WIDGET_H
#define WIDGET_H

#include <src/game/Gnumch.h>
#include <string>

using namespace std;

typedef enum {
    TEXT_LEFT,
    TEXT_CENTER,
    TEXT_RIGHT
} TextAlign;

class Clickable;
class TextButton;
class PicButton;
class Container;
class VBox;
class HBox;
class Column2;
class ButtonGroup;
class Label;
class PicLabel;
template<class T> class Spinner;
class TextField;

/// A small utility class for representing rectangles
class Rectangle {
    public:
        int x, y, w, h;

        Rectangle() {x=0; y=0; w=0; h=0;}
        Rectangle(int x_, int y_, int w_, int h_)
        { x = x_; y = y_; w = w_; h = h_; }

        SDL_Rect toSDL() const {
            return (SDL_Rect){x, y, w, h};
        }

        bool contains(int x_, int y_) const {
            return x_ >= x 
                && x_ <= (x+w) 
                && y_ >= y
                && y_ <= (y+h);
        }

        bool operator== (Rectangle param) const {
            return x == param.x
                && y == param.y
                && w == param.w
                && h == param.h;
        }

        bool operator!= (Rectangle param) const {
            return ! ((*this) == param);
        }

        bool operator>= (Rectangle param) const {
            return contains(param.x, param.y)
                && contains(param.x+param.w, param.y+param.h);
        }

        bool operator> (Rectangle param) const {
            return (*this) >= param
                && x != param.x
                && y != param.y
                && x+w != param.x + param.w
                && y+h != param.y + param.h;
        }

        bool operator< (Rectangle param) const {
            return param > (*this);
        }

        bool operator<= (Rectangle param) const {
            return param >= (*this);
        }

        Rectangle operator+ (Rectangle param) const {
            Rectangle ret;
            ret.x = min(x,param.x);
            ret.y = min(y,param.y);
            ret.w = max(x+w, param.x+param.w) - ret.x;
            ret.h = max(y+h, param.y+param.h) - ret.y;
            return ret;
        }

        void intersectSelf(const Rectangle &param) {
            int new_x = max(x, param.x);
            int new_y = max(y, param.y);
            w = max( min(x+w, param.x+param.w) - new_x, 0 );
            h = max( min(y+h, param.y+param.h) - new_y, 0 );
            x = new_x;
            y = new_y;
        }

        Rectangle intersect(const Rectangle &param) const {
            Rectangle ret(x, y, w, h);
            ret.intersectSelf(param);
            return ret;
        }
};

/// The parent class of just about everything.

class Widget {
    public:

        typedef enum {
            PRESS,
            RELEASE,
            CLICK
        } MouseEventType;

        typedef struct {
            int button;
            MouseEventType type;
        } MouseEvent_t;

        Widget();
        virtual ~Widget();

        /// Draw the widget to the screen.

        /** Containers draw their contents recursively. All other widgets just
         *  redraw their current contents.
         */
        virtual void draw(bool update=0) = 0;

        /// Remove the image of the widget from the screen.

        virtual void erase(bool update=0);

        /// Clean the widget's previously drawn position if it is different
        /// from the current position.
        void cleanDraw();

        /// Return the parent Widget.
        Container *getParent() {return parent;}

        /// Check the minimum required size for this widget.

        /** This function sets the values of w and h according to the minimum
         *  width and height of this widget. Setting a smaller width and/or
         *  height using setSize() will cause the picture to be scaled down.
         *  This could cause pictures to be obscured and text to be unreadable,
         *  so it is not a good idea.
         */
        virtual void getMinSize(int *w, int *h) = 0;

        /// Get the actual size of this Widget.

        /** If the size hasn't been set yet, this gives the same results as
         *  getMinSize(). If the size has been manually set, return that.
         */
        virtual void getSize(int *w, int *h);

        /// Handle a mouse click.

        /** Mouse clicks may not correspond directly to the underlying
         *  SDL_Events; the Menu class is in charge of translating them. If you
         *  want to get the SDL_Events themselves, set raw_events to true.
         */
        virtual void handleClick(const MouseEvent_t&);

        /// Pass up an event.

        /** This is used to pass SDL_Events up through Containers/Widgets until
         *  one is found that can do something with the event.
         */
        virtual bool handleEvent (const SDL_Event*);

        /// Handle a raw keyboard event.

        /** This is similar to handleEvent but for key presses instead of mouse
         * events. If this widget doesn't know what to do, it should return
         * false; it should not pass the event to a child or parent.
         *  \see handleEvent()
         *  \return whether the event was handled or not.
         */
        virtual bool handleRawKey (const SDL_Event*);

        /// Pass up a non-navigational key event.

        /** If a widget doesn't know how to handle this event, it will pass it
         *  to its parent.
         */
        virtual void handleKey (const SDL_Event*);

        /// Pass up/down a navigational key event.

        /** If sent to a Container, the focus will go to the first child after
         *  "from" in the given direction. Otherwise, the focus will go to the
         *  first sibling in the given direction.
         *
         *  \param direction 1 for down or -1 for up.
         *  \param from the Widget from which the focus change originates.
         */
        virtual Widget *handleKeyFocus(int direction, Widget *from);

        /// For a Container, return the child that is at the given coordinates.

        /** Otherwise, just return self.
         */
        virtual Widget *handleFocus(int x, int y) {return this;}

        /// Set the widget as focused.

        /** Generally, Widgets become focused when the mouse is positioned over
         *  them. Focused Widgets receive events and sometimes look different.
         *  \param update If true, redraw the widget.
         */
        virtual void focus (bool update=0);

        /// Set the widget as having keyboard focus.

        /** \see focus()
         */
        virtual void keyboardFocus (bool update=0);

        /// Set the widget as unfocused.

        /** \see focus()
         */
        virtual void unfocus (bool update=0);

        /// Set the widget as keyboard unfocused.

        /** \see keyboardFocus()
         *  \see unfocus()
         */
        virtual void keyboardUnfocus (bool update=0);

        /// Get the position.

        /** This function assumes that the widget has already been drawn.
         */
        void getPosition (Rectangle*);

        /// Set the position.

        virtual void setPosition (const Rectangle &);

        /// \see setPosition()
        virtual void setX(int x);

        /// \see setPosition()
        virtual void setY(int y);

        /// Set the size.

        /** The requested size should probably be bigger than the minimum size
         *  given by getMinSize(). If either parameter is set to -1, that
         *  parameter will be determined automatically.
         *
         *  This function is equivalent to
         *  \code setWidth(w), setHeight(h); \endcode
         *  \see getMinSize()
         */
        virtual void setSize (int w, int h);

        /// \see setSize()
        virtual void setWidth(int w);

        /// \see setSize()
        virtual void setHeight(int h);

        /// Check if the widget is focused.

        /** \return true if the Widget has focus.
         *  \see focus()
         *  \see unfocus()
         */
        bool hasFocus ();

        /// Check is the widget has keyboard focus.

        /** \see hasFocus()
         */
        bool hasKeyboardFocus ();

        /// Check if a point is inside a Widget

        /** This function assumes that the widget has already been positioned
         *  (ie. draw() has been called */

        bool contains(int x, int y);

        /// Set the parent Widget.
        void setParent(Container *p);

        /// Return whether to send this widget repeated click events for a
        /// held down button.
        bool getRepeating() {return repeating;}

        /// Set whether to send this widget repeated click events for a held
        /// down button.
        void setRepeating(bool b) {repeating = b;}

        /// Return whether this widget accepts raw events.

        /** Widgets that accept raw events receive events through the
         *  handleEvent() function rather that the handleClick() function.
         *  This is not user-settable because it depends only on the class.
         */
        bool getRawEvents() {return rawevents;}

        /// Return whether this widget accepts raw keyboard events.

        /** If not, this widget will not receive any events relating to
         *  navigation keys. Classes that return true should implement the
         *  handleRawKeyEvent() method instead of the handleKey() method.
         */
        bool getRawKeyEvents() {return rawkeyevents;}

        /// Return whether this widget accepts keyboard focus.

        /** Containers should generally have this set to true, or it will
         *  mess up keyboard focus for its children.
         */
        virtual bool getAllowKeyFocus() {return allow_key_focus;}

        /// Mark this widget (and all its parents) as changed.
        void setChanged(bool);

        /// Mark this widget (and all its children, if it is a Container) as
        /// changed.
        virtual void setAllChanged(bool b);

        /// Set this widget as highlighted.
        void setHighlight(bool);

    protected:
        Container *parent;
        Rectangle pos;
        Rectangle drawn_pos;

        bool focused;
        bool allow_key_focus;
        bool keyboard_focused;
        bool repeating;
        bool rawevents;
        bool rawkeyevents;
        bool changed;
        bool highlight;

        /* set this to true before handing a keyFocus up to a parent. If this
         * is true, DON'T hand the keyFocus to a parent or infinite recursion
         * may occur. */
        bool handling_key_focus;

        /// Resize this Widget.

        /** Also resize and reposition parents until everything is positioned
         *  nicely.
         */
        virtual void resize();

        /// Save the minimum size of this Widget.

        /** This should be called prior to changing the contents of the Widget:
         *  saveMinSize();
         *  <code that might change the widget's size>
         *  resize();
         */
        virtual void saveMinSize();

        int old_min_w, old_min_h;
};

/// An interface for all widgets with text

class TextWidget {
    public:
        /// Construct a TextWidget with a blank string.

        /** Note that the string will be translated if gettext is supported.
         */
        TextWidget();

        /// Construct a TextWidget with the given string.
        TextWidget(const string&);
        virtual ~TextWidget();

        /// return the current text
        /** Note that the string returned may not be the same as the string this
         *  widget was created with if the string is translatable.
         *  \returns the current text of this widget.
         */
        const char *getText();

        /// Set the text.

        /** Note that the string will be translated if gettext is supported.
         */
        void setText(const string&);

        /// Set the text align.

        /** \see TextAlign
         */
        void setTextAlign(TextAlign);

    protected:
        string text;
        TextAlign text_align;
        Rectangle drawn_text_pos;
};

#endif
