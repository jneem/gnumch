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
#include "ScoresMenu.h"
#include "Menus.h"
#include "../Gnumch.h" // for itostr
#include "../FileSys.h"

extern FileSys *fs;

ScoresMenu::ScoresMenu():
  back (_("Back"))
{
    vector<string> names;
    vector<int> scores;
    fs->readScores(names, scores);

    for (int i = 0; i < (int)names.size(); i++)
      {
	Label *name = new Label (names[i]);
	Label *score = new Label (itostr (scores[i]));
	labels.push_back (name);
	labels.push_back (score);
        scores_list.add (name, score);
      }
    if (names.empty ())
      {
	Label *name = new Label (_("No high scores yet"));
	Label *score = new Label (_("9999999999"));
	labels.push_back (name);
	labels.push_back (score);
	scores_list.add (name, score);
      }

    back.setCallback(popMenu, NULL);

    top.add (&scores_list);
    bottom.add (&back);

    top.setPack(Container::PACK_MIN_CENTER);
    bottom.setPack(Container::PACK_MIN_CENTER);
    scores_list.setAlign(Container::ALIGN_JUSTIFIED);

    add (&top);
    add (&bottom);
    setPack (Container::PACK_LEADING);
}

ScoresMenu::~ScoresMenu()
{
  for (int i = 0; i < (int)labels.size (); i++)
    delete labels[i];
}
