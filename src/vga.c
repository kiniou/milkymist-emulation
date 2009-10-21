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

#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include <hal/vga.h>

int vga_hres;
int vga_vres;
unsigned short int *vga_frontbuffer;
unsigned short int *vga_backbuffer;

static unsigned short int *surface_buffer;
static unsigned short int *extra_buffer;

static SDL_Surface *screen = NULL;

static void free_extrabuffer()
{
	free(extra_buffer);
}

void vga_init()
{
	vga_hres = 640;
	vga_vres = 480;

	screen = SDL_SetVideoMode(vga_hres, vga_vres, 16, SDL_HWPALETTE|SDL_SWSURFACE);
	if(!screen) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	if(SDL_MUSTLOCK(screen)) {
		fprintf(stderr, "SDL surface requires locking!\n");
		exit(1);
	}
	SDL_WM_SetCaption("Milkymist Video Output", NULL);

	vga_frontbuffer = surface_buffer = (unsigned short int *)screen->pixels;
	vga_backbuffer = extra_buffer = malloc(2*vga_hres*vga_vres);
	atexit(free_extrabuffer);

	printf("VGA: SDL emulation, %dx%d\n", vga_hres, vga_vres);
	printf("VGA: framebuffers at %p %p\n", surface_buffer, extra_buffer);
}

void vga_swap_buffers()
{
	memcpy(vga_frontbuffer, vga_backbuffer, 2*vga_hres*vga_vres);
	SDL_Flip(screen);
}

void vga_update()
{
	if(screen)
		SDL_Flip(screen);
}
