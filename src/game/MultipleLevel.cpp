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
#include "MultipleLevel.h"
#include "FileSys.h"
#include "ConfigFile.h"

#define MIN_LEVEL 2
#define MAX_LEVEL 20
#define DEF_MIN_LEVEL 2
#define DEF_MAX_LEVEL 12
#define MIN_MULTIPLIER 10
#define MAX_MULTIPLIER 20

extern FileSys *fs;
extern const char *number_error_fmt[];

MultipleConfig::MultipleConfig(): LevelConfig("multipleLevel")
{
    title = MultipleLevel::getTitle();
    description = MultipleLevel::getDescription();
    abs_maxlevel = MAX_LEVEL;
    abs_minlevel = MIN_LEVEL;
    def_maxlevel = DEF_MAX_LEVEL;
    def_minlevel = DEF_MIN_LEVEL;

    conf = fs->openConfig("gnumch.cfg");
    readConfig();
    delete conf;
    conf = NULL;
}

MultipleConfig::~MultipleConfig()
{
    conf = fs->openConfig("gnumch.cfg", true);
    writeConfig();
    delete conf;
    conf = NULL;
}

Level *MultipleConfig::makeLevel()
{
    return new MultipleLevel(this);
}

Container *MultipleConfig::makeConfigDialog()
{
    return new MultipleGUI(this);
}

bool MultipleConfig::sanityCheck()
{
    return max_multiplier >= MIN_MULTIPLIER && max_multiplier <= MAX_MULTIPLIER
                                            && LevelConfig::sanityCheck();
}

void MultipleConfig::readConfig()
{
    max_multiplier = conf->readInt(section, "max_multiplier", 12);
    LevelConfig::readConfig();
}

void MultipleConfig::writeConfig()
{
    conf->writeInt(section, "max_multiplier", max_multiplier);
}

MultipleLevel::MultipleLevel(const MultipleConfig *c): ListedNumberLevel(c)
{
}

const char *MultipleLevel::getError(const Number *num)
{
    vector<int> factors;
    size_t n;
    string replacement;

    getFactors(&factors, num->getValue());

    error = _("Oops! %X is not a multiple of %Y.\n");
    if (num->getValue() == 1) {
        error += _("The only factor of 1 is itself.");
    } else {
        error += _("The factors of %X are ")
               + translateList(factors) + ".";
    }
    n = 0;
    while ( (n=error.find("%X", n)) != string::npos ) {
        error.replace(n, 2, num->getText(), strlen(num->getText()));
    }
    while ( (n=error.find("%Y", n)) != string::npos ) {
        replacement = itostr(curlevel);
        error.replace(n, 2, replacement.c_str(), replacement.size());
    }
    return error.c_str();
}

void MultipleLevel::nextLevel()
{
    const MultipleConfig *c = static_cast<const MultipleConfig*>(conf);

    good_num.clear();
    bad_num.clear();

    curlevel++;
    if (curlevel > c->maxlevel) curlevel = c->minlevel;

    for (int i=1; i<=curlevel*c->max_multiplier; i++) {
        ( (i%curlevel)? bad_num : good_num ).push_back(i);
    }

    sprintf(&title, _("Multiples of %d"), curlevel);
}

MultipleGUI::MultipleGUI(MultipleConfig *c):
    LevelGUI(c),
    mult_spin(MAX_MULTIPLIER),
    mult_label(_("Maximum multiplier"))
{
    for (int i=MAX_MULTIPLIER-1; i>=MIN_MULTIPLIER; i--) {
        mult_spin.addItem(i);
    }
    mult_spin.setItem(MAX_MULTIPLIER - c->max_multiplier);

    options.add(&mult_label, &mult_spin);
    apply.setCallback(reinterpret_cast<Clickable::Callback>(applyChanges), this);
}

void MultipleGUI::applyChanges(Clickable *b, MultipleGUI *m)
{
    MultipleConfig *c = static_cast<MultipleConfig*>(m->conf);

    c->max_multiplier = m->mult_spin.getItem();
    LevelGUI::applyChanges(NULL, m);
}
