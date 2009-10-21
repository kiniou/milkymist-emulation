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

#include <termios.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <uart.h>

static struct termios otty;

static void restore_console()
{
	tcsetattr(0, TCSANOW, &otty);
}

void uart_async_init()
{
	struct termios ntty;
	
	tcgetattr(0, &otty);
	ntty = otty;
	ntty.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(0, TCSANOW, &ntty);
	atexit(restore_console);
}

void uart_async_isr_rx()
{
	printf("uart_async_isr_rx() should not be called in emulation mode\n");
}

void uart_async_isr_tx()
{
	printf("uart_async_isr_tx() should not be called in emulation mode\n");
}

void uart_force_sync(int f)
{
	/* do nothing */
}

void writechar(char c)
{
	write(0, &c, 1);
}

char readchar()
{
	char c;

	read(0, &c, 1);
	return c;
}

int readchar_nonblock()
{
	struct pollfd fd;
	int flags;
	int r;

	fd.fd = 0;
	fd.events = POLLIN;

	flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags|O_NONBLOCK);
	r = poll(&fd, 1, 0);
	fcntl(0, F_SETFL, flags);

	return r;
}
