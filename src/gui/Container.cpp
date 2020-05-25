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
#include "Container.h"
#include "Menu.h"

/// Create a Container with no children.

Container::Container()
{
    align = ALIGN_CENTER;
    pack  = PACK_MIN;
    padding = 1;
    callback = NULL;
    data = NULL;
    background = NULL;
    need_bg_redraw = true;

    handling_key_focus = false;
}

Container::~Container()
{
}

void Container::setBackground(SDL_Surface *s)
{
    background = s;
}

Widget *Container::handleFocus(int x, int y)
{
    vector<Widget*>::iterator wid, end = children.end();

    for (wid=children.begin(); wid<end; wid++) {
        if ( (*wid)->contains(x, y) ) {
            return (*wid)->handleFocus(x, y);
        }
    }
    return this;
}

void Container::handleClick(const MouseEvent_t &m)
{
    /* empty */
}

Widget *Container::handleKeyFocus(int direction, Widget *from)
{
    vector<Widget*>::iterator focpos =
            find(children.begin(), children.end(), from);
    focpos = nextKeyFocChild(focpos, direction);

    if (focpos < children.begin() || focpos >= children.end()) {
        if (from == NULL) {
            /* then we don't have focusable children and either:
             * 1) we were called directly from Menu:: for the first time or
             * 2) there aren't any focusable widgets in the tree
             */
            return this;
        }
        if (parent) {
            return parent->handleKeyFocus(direction, this);
        }
        focpos = nextKeyFocChild(focpos, direction);
        if (focpos < children.begin() || focpos >= children.end()) {
            return this;
        }
    }
    /* we are passing to a child, so they don't get a "from" pointer */
    return (*focpos)->handleKeyFocus(direction, NULL);
}

bool Container::getAllowKeyFocus()
{
    for (vector<Widget*>::iterator i=children.begin(); i<children.end(); i++) {
        if ((*i)->getAllowKeyFocus()) {
            return true;
        }
    }
    return false;
}

void Container::setCallback(Callback fn, void *fn_data)
{
    callback = fn;
    data = fn_data;
}

void Container::callCallback()
{
    if(callback) (*callback)(this, data);
}

void Container::add(Widget *child)
{
    children.push_back(child);
    if (child->getParent()) {
        printWarning("reparenting widget\n");
        child->getParent()->remove(child);
    }
    child->setParent(this);
    setChanged(1);
}

void Container::remove(Widget* child)
{
    vector<Widget*>::iterator i = find(children.begin(),
                                       children.end(), child);

    child->erase(1);
    if (i == children.end()) {
        printWarning("couldn't find child to remove\n");
    } else {
        (*i)->setParent(NULL);
        children.erase(i);
    }
    setChanged(1);
}

void Container::insert(Widget *child, Widget *before)
{
    vector<Widget*>::iterator i = find(children.begin(),
                                       children.end(), before);

    if (child->getParent()) {
        printWarning("reparenting widget\n");
        child->getParent()->remove(child);
    }
    if (i == children.end()) {
        printWarning("couldn't find child: inserting at end\n");
    }
    children.insert(i, child);
    child->setParent(this);
    setChanged(1);
}

void Container::clear()
{
    vector<Widget*>::iterator i;

    for (i=children.begin(); i<children.end(); i++) {
        (*i)->erase(1);
        (*i)->setParent(NULL);
    }
    children.clear();
    setChanged(1);
}

void Container::setAlign(Align new_align)
{
    align = new_align;
    setChanged(1);
}

void Container::setPack(PackMethod new_pack)
{
    pack = new_pack;
    setChanged(1);
}

void Container::setPadding(int pad)
{
    padding = pad;
    setChanged(1);
}

void Container::unfocus(bool update)
{
    for(unsigned i=0; i<children.size(); i++) {
        if (children[i]) children[i]->unfocus(update);
    }
}

void Container::keyboardUnfocus(bool update)
{
    for(unsigned i=0; i<children.size(); i++) {
        if (children[i]) children[i]->keyboardUnfocus(update);
    }
}

void Container::setWidth(int w_)
{
    pos.w = w_;
    if(w_ == -1) {
        vector<Widget*>::iterator wid, end=children.end();

        for(wid=children.begin(); wid<end; wid++)
            if(*wid) (*wid)->setWidth(-1);
    }
    setChanged(1);
}

void Container::setHeight(int h_)
{
    pos.h = h_;
    if(h_ == -1) {
        vector<Widget*>::iterator wid, end=children.end();

        for(wid=children.begin(); wid<end; wid++)
            if(*wid) (*wid)->setHeight(-1);
    }
    setChanged(1);
}

void Container::drawBackground(const Rectangle &r)
{
    Rectangle new_r = r.intersect(pos);

    if (new_r.w == 0 || new_r.h == 0) {
        return;
    }

    Rectangle clip(new_r.x - pos.x, new_r.y - pos.y, new_r.w, new_r.h);
    if (background) {
        Menu::drawPicClipped(background, clip, r);
    } else if (parent) {
        parent->drawBackground(r);
    } else {
        Menu::drawBox(r, false);
    }
    Menu::update(r);
}

void Container::setAllChanged(bool b)
{
    changed = b;
    need_bg_redraw = true;
    for (int i=0; i<(int)children.size(); i++) {
        if (children[i]) children[i]->setAllChanged(b);
    }
}

int Container::packMax(int total, int *size, int num)
{
    total -= padding*(num-1);
    int i;

    qsort(size, num, sizeof(int), intdescend);
    for(i=0; i<num && size[i]*(num-i) > total; i++) {
        total -= size[i];
    }
    if(i == num) {
        printWarning("couldn't fit children in packMax()\n");
        return total/num;
    }
    return total / (num-i);
}

/* return the position of a child, based on its alignment, between pos and
 * pos + dim, where the childs minimum dimension is child_dim */
int Container::childPos(int pos, int dim, int child_dim)
{
    int ret=0;
    switch(align) {
        case ALIGN_JUSTIFIED:
        case ALIGN_LEFT:
        case ALIGN_TOP:
            ret = pos;
            break;
        case ALIGN_CENTER:
            ret = pos + (dim - child_dim)/2;
            break;
        case ALIGN_RIGHT:
        case ALIGN_BOTTOM:
            ret = pos + dim - child_dim;
            break;
        default:
            cout << "Programming error: couldn't align child\n";
            break;
    }
    return ret;
}

/**
 * Fill a vector of Rectangles with the sizes of the children of a Container.
 * The Rectangles in the result will be filled out to be compatible with all
 * the packing/alignment/sizing functions. That is, they will be oriented as
 * if placed in a VBox.
 */
void Container::makeChildrenRects(vector<Rectangle> &dest, bool vertical)
{
    size_t i;
    int x1, x2;

    for (i=0; i<children.size(); i++) {
        if (vertical) {
            children[i]->getMinSize(&x1, &x2);
        } else {
            children[i]->getMinSize(&x2, &x1);
        }
        dest.push_back(Rectangle(0, 0, x1, x2));
    }
}

void Container::packChildren(vector<Rectangle> &dest, bool vertical)
{
    Rectangle self = pos;

    if (!vertical) {
        swap(self.x, self.y);
        swap(self.w, self.h);
    }

    switch (pack) {
        case PACK_MIN:
            packChildrenMin(dest, self, padding);
            break;
        case PACK_MIN_CENTER:
            packChildrenMinCenter(dest, self, padding);
            break;
        case PACK_EQUAL:
            packChildrenEqual(dest, self, padding);
            break;
        case PACK_MAX:
            packChildrenMax(dest, self, padding);
            break;
        case PACK_LEADING:
            packChildrenLeading(dest, self, padding);
            break;
        case PACK_TRAILING:
            packChildrenTrailing(dest, self, padding);
            break;
    }
}

void Container::packChildrenMin(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    int y_cur = self.y;
    vector<Rectangle>::iterator i;

    for (i=dest.begin(); i<dest.end(); i++) {
        i->y = y_cur;
        y_cur += i->h + padding;
    }
    assert( y_cur - padding <= self.y + self.h );
}

void Container::packChildrenMinCenter(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    int y_cur;
    int h_total;
    vector<Rectangle>::iterator i;

    h_total = 0;
    for (i=dest.begin(); i<dest.end(); i++) {
        h_total += i->h + padding;
    }
    h_total -= padding;

    y_cur = self.y + (self.h - h_total) / 2;
    for (i=dest.begin(); i<dest.end(); i++) {
        i->y = y_cur;
        y_cur += i->h + padding;
    }
    assert( y_cur - padding <= self.y + self.h );
}

void Container::packChildrenEqual(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    int y_cur = self.y;
    vector<Rectangle>::iterator i;
    int h_max = 0;

    for (i=dest.begin(); i<dest.end(); i++) {
        h_max = max(i->h, h_max);
    }
    for (i=dest.begin(); i<dest.end(); i++) {
        i->y = y_cur;
        i->h = h_max;
        y_cur += h_max + padding;
    }

    assert( y_cur - padding <= self.y + self.h );
}

void Container::packChildrenMax(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    int sorted[dest.size()];
    int y_cur;
    int h_avg;
    int h_total = self.h - (dest.size()-1) * padding;
    size_t i;
    const size_t size = dest.size();

    for (i=0; i<size; i++) {
        sorted[i] = dest[i].h;
    }
    sort(sorted, sorted + size);

    for (i=size-1; i>=0 && (int)(sorted[i]*(i+1)) > h_total; i--) {
        h_total -= sorted[i];
    }
    if (i < 0) {
        printWarning("couldn't fit all the children in packChildrenMax()\n");
        h_avg = sorted[0];
    } else {
        h_avg = h_total / (i+1);
    }

    /* do the actual positioning */
    y_cur = self.y;
    for (i=0; i<size; i++) {
        dest[i].y = y_cur;
        y_cur += max(dest[i].h, h_avg) + padding;
        dest[i].h = y_cur - dest[i].y - padding;
    }

    assert( y_cur - padding <= self.y + self.h );
}

void Container::packChildrenLeading(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    vector<Rectangle>::iterator i;
    int y_cur = self.y + self.h;

    for (i=dest.end()-1; i>dest.begin(); i--) {
        i->y = y_cur - i->h;
        y_cur -= i->h + padding;
    }
    dest[0].y = self.y;
    dest[0].h = y_cur - self.y;
}

void Container::packChildrenTrailing(vector<Rectangle> &dest, const Rectangle &self, int padding)
{
    vector<Rectangle>::iterator i;
    int y_cur = self.y;

    for (i=dest.begin(); i<dest.end()-1; i++) {
        i->y = y_cur;
        y_cur += i->h + padding;
    }
    dest[dest.size()-1].y = y_cur;
    dest[dest.size()-1].h = self.y + self.h - y_cur;

    assert( y_cur - padding <= self.y + self.h );
}

void Container::alignChild(int x, int w, int *x_aligned, int *w_aligned, Align align)
{
    switch(align) {
        case ALIGN_JUSTIFIED:
            *w_aligned = w;  // fall through
        case ALIGN_LEFT:
        case ALIGN_TOP:
            *x_aligned = x;
            break;
        case ALIGN_CENTER:
            *x_aligned = x + (w - *w_aligned) / 2;
            break;
        case ALIGN_RIGHT:
        case ALIGN_BOTTOM:
            *x_aligned = x + w - *w_aligned;
            break;
    }
}

void Container::alignChildren(vector<Rectangle> &dest, bool vertical)
{
    vector<Rectangle>::iterator i;
    Rectangle self = pos;

    if (!vertical) {
        swap(self.x, self.y);
        swap(self.w, self.h);
    }
    for (i=dest.begin(); i<dest.end(); i++) {
        alignChild(self.x, self.w, &i->x, &i->w, align);
    }
}

void Container::sizeChildren(const vector<Rectangle> &dest, Rectangle &result, bool vertical)
{
    if (pack == PACK_EQUAL) {
        sizeChildrenEqual(dest, result, padding);
    } else {
        sizeChildrenMin(dest, result, padding);
    }
    if (!vertical) {
        swap(result.w, result.h);
    }
}

void Container::sizeChildrenMin(const vector<Rectangle> &dest, Rectangle &result, int padding)
{
    vector<Rectangle>::const_iterator i;

    for (i=dest.begin(); i<dest.end(); i++) {
        result.h += i->h + padding;
        result.w = max(result.w, i->w);
    }
    result.h -= padding;
}

void Container::sizeChildrenEqual(const vector<Rectangle> &dest, Rectangle &result, int padding)
{
    vector<Rectangle>::const_iterator i;
    int max_h = 0;

    for (i=dest.begin(); i<dest.end(); i++) {
        max_h = max(i->h, max_h);
        result.w = max(result.w, i->w);
    }
    result.h = max_h * dest.size() + padding * (dest.size()-1);
}

void Container::swapOrientation(vector<Rectangle> &dest)
{
    vector<Rectangle>::iterator i;

    for (i=dest.begin(); i<dest.end(); i++) {
        swap(i->w, i->h);
        swap(i->x, i->y);
    }
}

bool Container::commonDraw()
{
    if (!changed) {
        return false;
    }

    if (drawn_pos.x != -1 && drawn_pos != pos) {
        /* then we just did a cleanDraw, so all children need to be redrawn */
        setAllChanged(1);
    }

    if (background && need_bg_redraw) {
        drawBackground(pos);
        need_bg_redraw = false;
    }
    drawn_pos = pos;
    setChanged(0);
    return true;
}

void Container::simpleDraw(bool vertical, bool update)
{
    if (!commonDraw()) {
        return;
    }

    vector<Rectangle> child_pos;
    Rectangle min_size(pos.x, pos.y, 0, 0);

    getMinSize(&min_size.w, &min_size.h);
    assert( min_size <= pos );

    makeChildrenRects(child_pos, vertical);
    packChildren(child_pos, vertical);
    alignChildren(child_pos, vertical);

    if (!vertical) {
        swapOrientation(child_pos);
    }

    /* draw children */
    for (size_t i=0; i<children.size(); i++) {
        children[i]->setPosition(child_pos[i]);
        children[i]->cleanDraw();
    }
    for (size_t i=0; i<children.size(); i++) {
        children[i]->draw(update);
    }
}

/* search the children vector for the next widget after start that takes
 * keyboard focus. inc controls the search direction.
 */
vector<Widget*>::iterator
Container::nextKeyFocChild(vector<Widget*>::iterator start, int inc) {
    if (start < children.begin() || start >= children.end()) {
        start = (inc == 1) ? children.begin() : children.end()-1;
    } else {
        start += inc;
    }
    while (start < children.end() && start >= children.begin()) {
        if ((*start)->getAllowKeyFocus()) {
            break;
        }
        start += inc;
    }
    return start;
}

void VBox::draw(bool update)
{
    simpleDraw(true, update);
}

void VBox::getMinSize(int *w_param, int *h_param)
{
    vector<Rectangle> child_pos;
    Rectangle result;

    makeChildrenRects(child_pos, true);
    sizeChildren(child_pos, result, true);
    *w_param = result.w;
    *h_param = result.h;
}

void HBox::draw(bool update)
{
    simpleDraw(false, update);
}

void HBox::getMinSize(int *w_param, int *h_param)
{
    vector<Rectangle> child_pos;
    Rectangle result;

    makeChildrenRects(child_pos, false);
    sizeChildren(child_pos, result, false);
    *w_param = result.w;
    *h_param = result.h;
}

/*__________________________________Column2___________________________________*/

void Column2::add(Widget *w1, Widget *w2)
{
    if(children.size() % 2) {
        children.push_back((Widget*)NULL);
    }
    children.push_back(w1);
    children.push_back(w2);
    w1->setParent(this);
    w2->setParent(this);
}

void Column2::draw(bool update)
{
    vector<Widget*>::iterator i;
    vector<Rectangle> child_pos;
    vector<Rectangle>::iterator r;
    int w_col;

    assert(children.size() % 2 == 0);

    if (!commonDraw()) {
        return;
    }

    for (i=children.begin(); i<children.end(); i+=2) {
        int w1=0, h1=0, w2=0, h2=0;
        if (*i) {
            (*i)->getMinSize(&w1, &h1);
        }
        if (*(i+1)) {
            (*(i+1))->getMinSize(&w2, &h2);
        }
        child_pos.push_back( Rectangle(0, 0, max(w1, w2)*2, max(h1,h2)) );
    }
    packChildren(child_pos, true);

    /* work out the column width */
    w_col = 0;
    if (align == ALIGN_JUSTIFIED) {
        w_col = pos.w / 2;
    } else {
        for (r=child_pos.begin(); r<child_pos.end(); r++) {
            w_col = max(w_col, r->w / 2);
        }
    }

    /* align children */
    for (size_t i=0; i<children.size(); i+= 2) {
        Rectangle chpos = child_pos[i/2];
        int dummy;
        if (children[i]) {
            children[i]->getMinSize(&chpos.w, &dummy);
            alignChild(pos.x, w_col, &chpos.x, &chpos.w, align);
            children[i]->setPosition(chpos);
            children[i]->cleanDraw();
        }
        if (children[i+1]) {
            children[i+1]->getMinSize(&chpos.w, &dummy);
            alignChild(pos.x+w_col, w_col, &chpos.x, &chpos.w, align);
            children[i+1]->setPosition(chpos);
            children[i+1]->cleanDraw();
        }
    }

    /* draw children */
    for (i=children.begin(); i<children.end(); i++) {
        if (*i) {
            (*i)->draw(update);
        }
    }
}

void Column2::getMinSize(int *w_param, int *h_param)
{
    int w_tmp, h_tmp, h_tmp_left, w_auto=0, h_auto=0, max_child_h=0;
    vector<Widget*>::iterator wid, end=children.end();

    w_auto = h_auto = h_tmp_left = max_child_h = 0;
    bool left = 1;
    for(wid=children.begin(); wid<end; wid++) {
        if(*wid) {
            (*wid)->getMinSize(&w_tmp, &h_tmp);
            w_auto = max(w_auto, w_tmp);
            max_child_h = max(max_child_h, h_tmp);
        } else {
            h_tmp = 0;
        }

        if(left) {
            h_tmp_left = h_tmp;
        } else {
            h_auto += max(h_tmp_left, h_tmp) + padding;
        }
        left = !left;
    }
    if(!left) {
        // then the most recent child was left, so we need to add its height
        h_auto += h_tmp;
    }
    if (pack == PACK_EQUAL) {
        *h_param = (max_child_h+padding)*((children.size()+1)/2) - padding;
    } else {
        *h_param = h_auto - padding;
    }
    *w_param = w_auto*2;
}

/*_________________________________ButtonGroup________________________________*/
ButtonGroup::ButtonGroup(bool vertical)
{
    foc = NULL;
    selected = NULL;
    if(vertical) box = new VBox();
    else         box = new HBox();

    Container::add(box);
}

ButtonGroup::~ButtonGroup()
{
    delete box;
}

void ButtonGroup::setWidth(int w_)
{
    box->setWidth(w_);
    pos.w = w_;
}

void ButtonGroup::setHeight(int h_)
{
    box->setHeight(h_);
    pos.h = h_;
}

void ButtonGroup::draw(bool update)
{
    if (!commonDraw()) {
        return;
    }

    box->setPosition(pos);
    box->draw(update);
}

void ButtonGroup::getMinSize(int *w_param, int *h_param)
{
    box->getMinSize(w_param, h_param);
}

void ButtonGroup::add(Widget *bad)
{
    printWarning("trying to add a non-Clickable Widget to a ButtonGroup\n");
}

void ButtonGroup::add(Clickable *child, Clickable::Callback call, void *d)
{
    if (!selected) {
        selected = child;
        child->setHighlight(1);
    }
    buttons.push_back(child);
    callbacks.push_back(call);
    data.push_back(d);
    child->setCallback((Clickable::Callback)(buttonCallback), this);
    box->add(child);
}

void ButtonGroup::buttonCallback(Clickable *b, ButtonGroup *g)
{
    int i;
    for (i=0; i<(int)g->buttons.size() && g->buttons[i] != b; i++)
        ;
    assert(i != (int)g->buttons.size());

    b->setHighlight(1);
    g->selected->setHighlight(0);
    g->selected = b;
    if (g->callbacks[i]) {
        (g->callbacks[i])(g->buttons[i], g->data[i]);
    }
}
