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
#include <Level.h>
#include <Game.h>
#include <FileSys.h>
#include <ConfigFile.h>
#include <PrimeLevel.h>
#include <FactorLevel.h>
#include <MultipleLevel.h>
#include <EqualityLevel.h>

extern FileSys *fs;

LevelConfig::LevelConfig(const char *s):
    abs_maxlevel(10),   // these 4 should really be set by the subclass, but just in case
    def_maxlevel(10),
    abs_minlevel(5),
    def_minlevel(5),
    section(s)
{
}

bool LevelConfig::sanityCheck()
{
    return (minlevel >= abs_minlevel && maxlevel <= abs_maxlevel
                                     && minlevel <= maxlevel);
}

void LevelConfig::readConfig()
{
    maxlevel = conf->readInt(section, "max", def_maxlevel);
    minlevel = conf->readInt(section, "min", def_minlevel);
}

void LevelConfig::writeConfig()
{
    conf->writeInt(section, "max", maxlevel);
    conf->writeInt(section, "min", minlevel);
}

Container *LevelConfig::makeConfigDialog()
{
    return new LevelGUI(this);
}

ExpressionLevelConfig::ExpressionLevelConfig(const char *s):
    LevelConfig(s)
{
}

bool ExpressionLevelConfig::sanityCheck()
{
    return maxsub >= EXPR_MIN_SUBTRACTOR
        && maxsub <= EXPR_MAX_SUBTRACTOR
        && maxdiv >= EXPR_MIN_DIVISOR
        && maxdiv <= EXPR_MAX_DIVISOR
        && LevelConfig::sanityCheck();
}

void ExpressionLevelConfig::readConfig()
{
    maxsub = conf->readInt(section, "maxsub", 5);
    maxdiv = conf->readInt(section, "maxdiv", 5);
    op[ADD] = conf->readInt(section, "op_add", 1);
    op[SUB] = conf->readInt(section, "op_sub", 1);
    op[MUL] = conf->readInt(section, "op_mul", 1);
    op[DIV] = conf->readInt(section, "op_div", 1);
    LevelConfig::readConfig();
}

void ExpressionLevelConfig::writeConfig()
{
    conf->writeInt(section, "maxsub", maxsub);
    conf->writeInt(section, "maxdiv", maxdiv);
    conf->writeInt(section, "op_add", op[ADD]);
    conf->writeInt(section, "op_sub", op[SUB]);
    conf->writeInt(section, "op_mul", op[MUL]);
    conf->writeInt(section, "op_div", op[DIV]);
}

Level::Level(const LevelConfig *c):
    conf(c)
{
    curlevel = c->maxlevel;
}

Level::~Level()
{
}

const char *numlist_separator()
{
    return ", ";
}
const char *numlist_final_separator()
{
    return _(" and ");
}

extern Game *game;
extern FileSys *fs;

Number::Number(const string &text_, bool isgood_, int value_)
{
    text = text_;
    isgood = isgood_;
    value = value_;
    pic = NULL;
}

Number::~Number()
{
    if(pic) SDL_FreeSurface(pic);
}

void Number::render()
{
    if(pic) SDL_FreeSurface(pic);
    pic = game->renderText(text.c_str(), FONT_SQUARE);
}

void Number::unrender()
{
    if(pic) SDL_FreeSurface(pic);
    pic = NULL;
}

SDL_Surface *Number::getPic()
{
    return pic;
}

/*___________________________________level____________________________________*/

void Level::getLevelList(vector<LevelConfig*> *vec)
{
    vec->clear();
    vec->push_back(new PrimeConfig());
    vec->push_back(new FactorConfig());
    vec->push_back(new MultipleConfig());
    vec->push_back(new EqualityConfig());
    vec->push_back(new InequalityConfig());
}

void Level::releaseNumber(Number *num)
{
    assert(num);

    map<Number*, int, ltnum>::iterator i;
    i = numbers.find(num);
    assert(i != numbers.end());

    if (i->second == 1) {
        delete i->first;
        numbers.erase(i);
    } else {
        i->second--;
    }
}

void Level::reRender()
{
    map<Number*, int, ltnum>::iterator i;

    for (i=numbers.begin(); i != numbers.end(); i++) {
        i->first->render();
    }
}

Number *Level::addNumber(const char *text, bool good, int value)
{
    map<Number*, int, ltnum>::iterator i;
    Number *ret = new Number(text, good, value);

    i = numbers.lower_bound(ret);
    if ( i != numbers.end() && !strcmp(text, i->first->getText())) {
        i->second++;
        delete ret;
        ret = i->first;
    } else {
        numbers[ret] = 1;
        ret->render();
    }
    return ret;
}

string Level::translateList(const vector<int> &list)
{
    bool prev = false;
    string str;

    for (int i=0; i<(int)list.size()-1; i++) {
        if (prev) {
            str += numlist_separator();
        }
        str += itostr(list[i]);

        prev = true;
    }
    if (prev) str += numlist_final_separator();
    str += itostr(list[list.size()-1]);
    return str;
}

void Level::getFactors(vector<int> *vec, int n)
{
    int max = n/2 + 1;

    vec->clear();
    vec->push_back(1);
    for (int i=2; i<max; i++) {
        if (n%i == 0) {
            vec->push_back(i);
        }
    }
    vec->push_back(n);
}

LevelGUI::LevelGUI(LevelConfig *c):
    min_spin(c->abs_maxlevel),
    max_spin(c->abs_maxlevel),
    apply(_("Apply")),
    cancel(_("Cancel")),
    min_label(_("Minimum level")),
    max_label(_("Maximum level")),
    conf(c)
{
    for (int i=c->abs_maxlevel-1; i >= c->abs_minlevel; i--) {
        min_spin.addItem(i);
        max_spin.addItem(i);
        min_spin.setCallback((Spinner<int>::Callback)minSpinCallback, &max_spin);
        max_spin.setCallback((Spinner<int>::Callback)maxSpinCallback, &min_spin);
    }
    min_spin.setItem(c->abs_maxlevel - c->minlevel);
    max_spin.setItem(c->abs_maxlevel - c->maxlevel);

    apply.setCallback((Clickable::Callback)applyChanges, this);
    cancel.setCallback(popMenu, NULL);

    options.add(&min_label, &min_spin);
    options.add(&max_label, &max_spin);

    this->add(&top);
    this->add(&bottom);
    top.add(&options);
    bottom.add(&bottom_l);
    bottom.add(&bottom_r);
    bottom_l.add(&cancel);
    bottom_r.add(&apply);

    this->setPack(Container::PACK_LEADING);
    this->setAlign(Container::ALIGN_JUSTIFIED);
    top.setPack(Container::PACK_MIN_CENTER);
    bottom.setPack(Container::PACK_MAX);
    bottom.setAlign(Container::ALIGN_BOTTOM);
    bottom_l.setAlign(Container::ALIGN_LEFT);
    bottom_r.setAlign(Container::ALIGN_RIGHT);
    options.setAlign(Container::ALIGN_JUSTIFIED);
}

void LevelGUI::applyChanges(Clickable *a, LevelGUI *g)
{
    g->conf->minlevel = g->min_spin.getItem();
    g->conf->maxlevel = g->max_spin.getItem();
    popMenu();
}

Number *ListedNumberLevel::randomNumber()
{
    int val;
    bool good;

    if (rand()%2) {
        good = false;
        val = bad_num[ RANDLT(bad_num.size()) ];
    } else {
        good = true;
        val = good_num[ RANDLT(good_num.size()) ];
    }

    string text = itostr(val);
    return addNumber(text.c_str(), good, val);
}

const ExpressionLevel::GetExpr ExpressionLevel::getExprFns[] = {
    ExpressionLevel::getAddExpr,
    ExpressionLevel::getSubExpr,
    ExpressionLevel::getMulExpr,
    ExpressionLevel::getDivExpr
};

ExpressionLevelGUI::ExpressionLevelGUI(ExpressionLevelConfig *c):
    LevelGUI(c),
    maxsub_spin(EXPR_MAX_SUBTRACTOR),
    maxdiv_spin(EXPR_MAX_DIVISOR),
    lab_maxsub(_("Maximum subtractor")),
    lab_maxdiv(_("Maximum divisor"))
{
    for (int i=EXPR_MAX_SUBTRACTOR-1; i >= EXPR_MIN_SUBTRACTOR; i--) {
        maxsub_spin.addItem(i);
    }
    for (int i=EXPR_MAX_DIVISOR-1; i >= EXPR_MIN_DIVISOR; i--) {
        maxdiv_spin.addItem(i);
    }
    maxsub_spin.setItem(EXPR_MAX_SUBTRACTOR - c->maxsub);
    maxdiv_spin.setItem(EXPR_MAX_DIVISOR - c->maxdiv);

    lab_op[ADD].setText(_("Addition"));
    lab_op[SUB].setText(_("Subtraction"));
    lab_op[MUL].setText(_("Multiplication"));
    lab_op[DIV].setText(_("Division"));

    for (int i=ADD; i <= DIV; i++) {
        tog_op[i].setText( c->op[i] ? _("On") : _("Off") );
        tog_op[i].setCallback((Clickable::Callback)toggleOps, this);
        options.add(&lab_op[i], &tog_op[i]);
    }

    options.add(&lab_maxsub, &maxsub_spin);
    options.add(&lab_maxdiv, &maxdiv_spin);

    apply.setCallback(reinterpret_cast<Clickable::Callback>(applyChanges), this);
}

void ExpressionLevelGUI::toggleOps(TextButton *c, ExpressionLevelGUI *e)
{
    int totalon = 0;

    /* don't allow them to turn off all 4 at once */
    for (int i=ADD; i <= DIV; i++)
        totalon += (strcmp(e->tog_op[i].getText(), _("On")) == 0);
    if (!strcmp(c->getText(), _("On")) && totalon == 1) {
        return;
    }
    c->setText( (strcmp(c->getText(), _("On")) == 0) ? _("Off") : _("On") );
}

void ExpressionLevelGUI::applyChanges(Clickable *a, ExpressionLevelGUI *g)
{
    ExpressionLevelConfig *c = static_cast<ExpressionLevelConfig*>(g->conf);
    c->maxsub = g->maxsub_spin.getItem();
    c->maxdiv = g->maxdiv_spin.getItem();
    for (int i=ADD; i < DIV; i++)
        c->op[i] = (strcmp(g->tog_op[i].getText(), _("On")) == 0);
    LevelGUI::applyChanges(NULL, g);
}

ExpressionLevel::ExpressionLevel(const ExpressionLevelConfig *c): Level(c)
{
}

ExpressionLevel::~ExpressionLevel()
{
}

void ExpressionLevel::getExpr(int val)
{
    const ExpressionLevelConfig *c = static_cast<const ExpressionLevelConfig*>(conf);
    int numops = c->op[ADD] + c->op[SUB] + c->op[MUL] + c->op[DIV];
    assert(numops>0);

    int max = RAND_MAX - RAND_MAX%numops;
    int r = rand() % max;
    int thresh = max/numops;
    for (int i=ADD; i<=DIV; i++)
      {
        if (c->op[i])
	  {
	    if (r < thresh)
	      {
		(*getExprFns[i])(&expr, val, this);
		return;
	      }
	    else
	      r -= thresh;
	  }
      }
    assert(false);
}

void ExpressionLevel::getAddExpr(string *str, int val, const ExpressionLevel *l)
{
    int op1 = RANDLE(val);
    int op2 = val-op1;

    sprintf(str, "%d + %d", op1, op2);
}

void ExpressionLevel::getSubExpr(string *str, int val, const ExpressionLevel *l)
{
    int op2 = RANDLE(static_cast<const ExpressionLevelConfig*>(l->conf)->maxsub);
    int op1 = val + op2;

    sprintf(str, "%d \xe2\x88\x92 %d", op1, op2); /* utf-8 minus sign */
}

void ExpressionLevel::getMulExpr(string *str, int val, const ExpressionLevel *l)
{
    vector<int> factors;

    getFactors(&factors, val);
    int op1 = factors[RANDLT(factors.size()-1) + 1];
    int op2 = (op1 == 0) ? RANDLE (l->curlevel) : val/op1;

    sprintf(str, "%d × %d", op1, op2);
}

void ExpressionLevel::getDivExpr(string *str, int val, const ExpressionLevel *l)
{
    int op2 = RANDLT(static_cast<const ExpressionLevelConfig*>(l->conf)->maxdiv) + 1;
    int op1 = val * op2;

    sprintf(str, "%d ÷ %d", op1, op2);
}

void minSpinCallback(Spinner<int>* smaller, Spinner<int> *bigger)
{
    if (smaller->getItem() > bigger->getItem()) {
        bigger->setItem(smaller->getIndex());
    }
}

void maxSpinCallback(Spinner<int>* bigger, Spinner<int>* smaller)
{
    minSpinCallback(smaller, bigger);
}

