/*
 * Milkymist Emulation Libraries
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL/SDL.h>
#include <system.h>
#include <vga.h>

void emu_init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Audio/Video initialization failed: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
}

void emu_service()
{
	SDL_Event e;

	if(SDL_PollEvent(&e)) {
		switch(e.type) {
			case SDL_QUIT:
				exit(0);
				break;
		}
	}
}

void flush_bridge_cache()
{
	vga_update();
}
