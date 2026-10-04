/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Game.hpp"
#include "GameGlobals.h"
#include "GameState.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Level/Dialog.hpp"
#include "WindowContext.hpp"

#include <SDL_thread.h>

const char* pathtypenames[] = { "keepwalking", "pause" };
const char* editortypenames[] = {
    "active", "sitting", "sitting wall", "sleeping",
    "dead1", "dead2", "dead3", "dead4"
};

void Game::fireSound(int sound)
{
    emit_sound_at(sound);
}

void Game::inputText(std::string& str, unsigned* charselected, GameState& gamestate)
{
    SDL_Event evenement;

    if (!waiting) {
        SDL_StartTextInput();
        waiting = true;
    }

    while (SDL_PollEvent(&evenement)) {
        if (!sdlEventProc(evenement)) {
            gamestate.tryquit = 1;
            break;
        }
        switch (evenement.type) {
            case SDL_TEXTEDITING:
                /* FIXME - We should handle this for complete input method support */
                break;
            case SDL_TEXTINPUT:
                str.insert(*charselected, evenement.text.text);
                (*charselected) += strlen(evenement.text.text);
                break;
            case SDL_KEYDOWN:
                if (evenement.key.keysym.sym == SDLK_ESCAPE) {
                    str.clear();
                    *charselected = 0;
                    waiting = false;
                } else if (evenement.key.keysym.sym == SDLK_BACKSPACE) {
                    if ((*charselected) > 0) {
                        (*charselected)--;
                        str.erase(*charselected, 1);
                    }
                } else if (evenement.key.keysym.sym == SDLK_DELETE) {
                    if ((*charselected) < str.size()) {
                        str.erase(*charselected, 1);
                    }
                } else if (evenement.key.keysym.sym == SDLK_HOME) {
                    (*charselected) = 0;
                } else if (evenement.key.keysym.sym == SDLK_END) {
                    (*charselected) = str.size();
                } else if (evenement.key.keysym.sym == SDLK_LEFT) {
                    if ((*charselected) != 0) {
                        (*charselected)--;
                    }
                } else if (evenement.key.keysym.sym == SDLK_RIGHT) {
                    if ((*charselected) < str.size()) {
                        (*charselected)++;
                    }
                } else if (evenement.key.keysym.sym == SDLK_RETURN) {
                    waiting = false;
                }
                break;
        }
    }

    if (!waiting) {
        SDL_StopTextInput();
    }
}
