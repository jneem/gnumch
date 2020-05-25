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
#include <EqualityLevel.h>
#include <ConfigFile.h>
#include <FileSys.h>
#include "menus/Menus.h"

#define MAX_ERROR 6
#define MIN_ERROR 1
#define MIN_LEVEL 3
#define MAX_LEVEL 30
#define DEF_MIN_LEVEL 4
#define DEF_MAX_LEVEL 20

extern FileSys *fs;

EqualityConfig::EqualityConfig(const char *s): ExpressionLevelConfig(s)
{
    abs_minlevel = MIN_LEVEL;
    abs_maxlevel = MAX_LEVEL;
    def_maxlevel = DEF_MAX_LEVEL;
    def_minlevel = DEF_MIN_LEVEL;
    title = EqualityLevel::getTitle();
    description = EqualityLevel::getDescription();

    conf = fs->openConfig("gnumch.cfg");
    readConfig();
    delete conf;
    conf = NULL;
}

EqualityConfig::~EqualityConfig()
{
    conf = fs->openConfig("gnumch.cfg", true);
    writeConfig();
    delete conf;
    conf = NULL;
}

Level *EqualityConfig::makeLevel()
{
    return new EqualityLevel(this);
}

Container *EqualityConfig::makeConfigDialog()
{
    return new EqualityGUI(this);
}

bool EqualityConfig::sanityCheck()
{
    return max_error >= MIN_ERROR && max_error <= MAX_ERROR
                                  && ExpressionLevelConfig::sanityCheck();
}

void EqualityConfig::readConfig()
{
    max_error = conf->readInt(section, "max_error", 4);
    ExpressionLevelConfig::readConfig();
}

void EqualityConfig::writeConfig()
{
    conf->writeInt(section, "max_error", max_error);
    ExpressionLevelConfig::writeConfig();
}

InequalityConfig::InequalityConfig(): EqualityConfig("inequalityLevel")
{
    title = InequalityLevel::getTitle();
    description = InequalityLevel::getDescription();
}

Level *InequalityConfig::makeLevel()
{
    return new InequalityLevel(this);
}

EqualityLevel::EqualityLevel(const EqualityConfig *c):
    ExpressionLevel(c)
{
}

Number *EqualityLevel::randomNumber()
{
    const EqualityConfig *c = static_cast<const EqualityConfig*>(conf);
    int val;
    bool good;

    if (rand() > RAND_MAX/2) {
        val = curlevel;
        good = true;
    } else {
        int error = RANDLT(c->max_error)+1;
        if (rand() > RAND_MAX/2) {
            val = curlevel+error;
        } else {
            val = curlevel-error;
        }
        good = false;
    }
    getExpr(val);
    return addNumber(expr.c_str(), good, val);
}

const char *EqualityLevel::getError(const Number *num)
{
    error = _("Oops! %X = %Y");

    string s1 = num->getText();
    string s2 = itostr(num->getValue());
    error.replace(error.find("%X"), 2, s1.c_str(), s1.size());
    error.replace(error.find("%Y"), 2, s2.c_str(), s2.size());
    return error.c_str();
}

void EqualityLevel::nextLevel()
{
    const EqualityConfig *c = static_cast<const EqualityConfig*>(conf);
    assert(numbers.empty());
    curlevel++;
    if (curlevel > c->maxlevel) curlevel = c->minlevel;

    sprintf(&title, _("Equal to %d"), curlevel);
}

InequalityLevel::InequalityLevel(const EqualityConfig *c):
    ExpressionLevel(c)
{
}

Number *InequalityLevel::randomNumber()
{
    const EqualityConfig *c = static_cast<const EqualityConfig*>(conf);
    int val;
    bool good;

    if (rand() > RAND_MAX/2) {
        val = curlevel;
        good = false;
    } else {
        int error = RANDLT(c->max_error)+1;
        if (rand() > RAND_MAX/2) {
            val = curlevel+error;
        } else {
            val = max(curlevel-error, 1);
        }
        good = true;
    }
    getExpr(val);
    return addNumber(expr.c_str(), good, val);
}

const char *InequalityLevel::getError(const Number *num)
{
    error = _("Oops! %X = %Y");

    string s1 = itostr(num->getValue());
    string s2 = itostr(curlevel);
    error.replace(error.find("%X"), 2, s1.c_str(), s1.size());
    error.replace(error.find("%Y"), 2, s2.c_str(), s2.size());
    return error.c_str();
}

void InequalityLevel::nextLevel()
{
    const EqualityConfig *c = static_cast<const EqualityConfig*>(conf);
    curlevel++;
    if (curlevel > c->maxlevel) curlevel = c->minlevel;
    sprintf(&title, _("Not equal to %d"), curlevel);
}

EqualityGUI::EqualityGUI(EqualityConfig *c):
    ExpressionLevelGUI(c),
    lab_error(_("Maximum error")),
    spin_error(MAX_ERROR)
{
    for (int i=MAX_ERROR-1; i >= MIN_ERROR; i--) {
        spin_error.addItem(i);
    }
    spin_error.setItem(MAX_ERROR - c->max_error);
    options.add(&lab_error, &spin_error);
    apply.setCallback(reinterpret_cast<Clickable::Callback>(applyChanges), this);
}

void EqualityGUI::applyChanges(Clickable *a, EqualityGUI *g)
{
    EqualityConfig *c = static_cast<EqualityConfig*>(g->conf);
    c->max_error = g->spin_error.getItem();
    ExpressionLevelGUI::applyChanges(NULL, g);
}
