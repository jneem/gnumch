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
#ifndef GMENU_OPTIONSMENU_H
#define GMENU_OPTIONSMENU_H

#include <libGui.h>
#include "../Game.h"

void cancelChanges(Clickable*, void*);
void applyChanges(Clickable*, void*);
void applyInGameChanges(Clickable*, void*);

void showGamePane(Clickable*, void *menu);
void showGraphicsPane(Clickable*, void *menu);

class GraphicsOptionsPane: public Column2 {
    public:
        GraphicsOptionsPane();
        virtual ~GraphicsOptionsPane();

        static void applyChanges(GraphicsOptionsPane*);

    private:
        Spinner<int> bpp_spin;
        Spinner<SDL_Rect> size_spin;
        Label size, bpp, fullscreen;
        TextButton fullscreen_toggle;

        static void toggleFS(Clickable *button, GraphicsOptionsPane*);
};

class SoundOptionsPane: public Column2 {
    public:
        SoundOptionsPane();
        virtual ~SoundOptionsPane();

        static void applyChanges(SoundOptionsPane*);

    private:
        Spinner<int> fx_spin;
        Spinner<int> music_spin;
        Spinner<int> freq_spin;
        TextButton stereo_toggle;
        Label fx, music, freq, stereo;

        int channels;

        static void toggleStereo(Clickable*, SoundOptionsPane*);
        static int freqToIndex(int);
};

class GameOptionsPane: public Column2 {
    public:
        GameOptionsPane();
        virtual ~GameOptionsPane();

        static void applyChanges(GameOptionsPane*);
    private:
        vector<string> speeds;
        Label trognum, trogspeed, gamespeed, boardwidth, boardheight;
        Spinner<int> trognum_spin;
        Spinner<int> boardwidth_spin, boardheight_spin;
        Spinner<string> trogspeed_spin, gamespeed_spin;
};

class InputOptionsPane: public HBox {
    public:
        InputOptionsPane();
        virtual ~InputOptionsPane();

        static void applyChanges(InputOptionsPane*);

    private:
        KeyBindings b1, b2;

        VBox list1, list2;
        Column2 col1, col2;
        Label player1, player2;
        Label label1[KEY_NUM], label2[KEY_NUM];
        TextButton key1[KEY_NUM], key2[KEY_NUM];

        static void changeKey(TextButton *button, InputOptionsPane *in);
};

class TroggleOptionsPane: public Column2 {
    public:
        TroggleOptionsPane();
        virtual ~TroggleOptionsPane();

        static void applyChanges(TroggleOptionsPane*);

    private:
        PicLabel trog[5];
        TextButton toggle[5];

        int cur_mask;
        static void toggleButton(TextButton*, TroggleOptionsPane*);
};

class InGameOptionsMenu: public VBox {
    public:
        InGameOptionsMenu();
        virtual ~InGameOptionsMenu();

        static void applyChanges(Clickable*, InGameOptionsMenu*);

    protected:
        HBox top_mid;
        ButtonGroup choose_pane;
        TextButton apply, cancel;

    private:
        VBox top, bottom_l, bottom_r;
        HBox bottom;
        TextButton choose_graphics, choose_sound, choose_input;
        GraphicsOptionsPane graphics_pane;
        SoundOptionsPane sound_pane;
        InputOptionsPane input_pane;

        static void showGraphicsPane(Clickable*, InGameOptionsMenu*);
        static void showSoundPane(Clickable*, InGameOptionsMenu*);
        static void showInputPane(Clickable*, InGameOptionsMenu*);
};

/* this is the same as InGameOptions menu, but with the addition of a
 * couple more panes */
class OptionsMenu: public InGameOptionsMenu {
    public:
        OptionsMenu();
        virtual ~OptionsMenu();

        static void applyChanges(Clickable*, OptionsMenu*);

    private:
        GameOptionsPane game_pane;
        TroggleOptionsPane trog_pane;
        TextButton choose_game, choose_trog;

        static void showGamePane(Clickable*, OptionsMenu*);
        static void showTrogPane(Clickable*, OptionsMenu*);
};

#endif
