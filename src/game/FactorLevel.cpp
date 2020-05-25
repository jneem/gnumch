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
#include "FactorLevel.h"
#include "FileSys.h"
#include "ConfigFile.h"

#define MIN_LEVEL 4
#define MAX_LEVEL 200
#define DEF_MIN_LEVEL 4
#define DEF_MAX_LEVEL 48

#define MIN_FACTORS 2
#define MAX_FACTORS 4

extern FileSys *fs;
extern const char *number_error_fmt[];

FactorConfig::FactorConfig():
    LevelConfig("factorLevel")
{
    title = FactorLevel::getTitle();
    description = FactorLevel::getDescription();
    abs_maxlevel = MAX_LEVEL;
    abs_minlevel = MIN_LEVEL;
    def_maxlevel = DEF_MAX_LEVEL;
    def_minlevel = DEF_MIN_LEVEL;

    conf = fs->openConfig("gnumch.cfg");
    readConfig();
    delete conf;
    conf = NULL;
}

FactorConfig::~FactorConfig()
{
    conf = fs->openConfig("gnumch.cfg", true);
    writeConfig();
    delete conf;
    conf = NULL;
}

Level *FactorConfig::makeLevel()
{
    return new FactorLevel(this);
}

Container *FactorConfig::makeConfigDialog()
{
    return new FactorGUI(this);
}

bool FactorConfig::sanityCheck()
{
    return min_num_factors >= MIN_FACTORS && min_num_factors <= MAX_FACTORS
                                          && LevelConfig::sanityCheck();
}

void FactorConfig::readConfig()
{
    min_num_factors = conf->readInt(section, "min_num_factors", MIN_FACTORS);
    LevelConfig::readConfig();
}

void FactorConfig::writeConfig()
{
    conf->writeInt(section, "min_num_factors", min_num_factors);
    LevelConfig::writeConfig();
}

FactorLevel::FactorLevel(const FactorConfig *c): ListedNumberLevel(c)
{
}

const char *FactorLevel::getError(const Number *num)
{
    vector<int> multiples;
    size_t n;
    string replacement;
    getMultiples(&multiples, num->getValue());

    error = _("Oops! %X is not a factor of %Y.\n"
              "Multiples of %X include ")
          + translateList(multiples) + ".";
    replacement = itostr(curlevel);
    n = 0;
    while ((n = error.find("%X", 0)) != string::npos) {
        error.replace(n, 2, num->getText(), strlen(num->getText()));
    }

    n = error.find("%Y", 0);
    error.replace(n, 2, replacement.c_str(), replacement.size());

    return error.c_str();
}

void FactorLevel::nextLevel()
{
    const FactorConfig *c = static_cast<const FactorConfig*>(conf);
    good_num.clear();
    bad_num.clear();

    for (curlevel++; numFactors(curlevel) < c->min_num_factors; curlevel++)
        ;

    if (curlevel > c->maxlevel) {
        curlevel = c->minlevel;
    }

    for (int i=1; i<=curlevel; i++) {
        ( (curlevel%i) ? bad_num : good_num ).push_back(i);
    }

    sprintf(&title, _("Factors of %d"), curlevel);
}

void FactorLevel::getMultiples(vector<int> *mult, int fac)
{
    int tmp;

    mult->clear();
    for (tmp=2*fac; tmp+2*fac < curlevel; tmp+=fac)
        ;

    for (int i=0; i<4; i++, tmp += fac) {
        mult->push_back(tmp);
    }
}

int FactorLevel::numFactors(int n)
{
    int ret = 2;
    int max = n/2+1;
    for (int i=2; i<max; i++) {
        if (n%i == 0) {
            ret++;
        }
    }
    return ret;
}

FactorGUI::FactorGUI(FactorConfig *c):
    LevelGUI(c),
    min_factors_label(_("Minimum number of factors")),
    min_factors_spin(MAX_FACTORS)
{
    for (int i=MAX_FACTORS-1; i >= MIN_FACTORS; i--) {
        min_factors_spin.addItem(i);
    }
    min_factors_spin.setItem(MAX_FACTORS - c->min_num_factors);

    options.add(&min_factors_label, &min_factors_spin);
    apply.setCallback((Clickable::Callback)applyChanges, this);
}

void FactorGUI::applyChanges(Clickable *b, FactorGUI *f)
{
    FactorConfig *c = static_cast<FactorConfig*>(f->conf);

    c->min_num_factors = f->min_factors_spin.getItem();
    LevelGUI::applyChanges(NULL, f);
}
