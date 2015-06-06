/* This code is is taken from the iDoom source, which is released under the GPL
   The complete source code is available at: http://idoom.hyarion.com
   Thanks to hyarion and Jobbe for writing this code and letting me use it
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/kd.h>

#include "mvpd.h"


/*
 *  This file contains functions to handle key events from the iPod.
 *
 *  Copyright 2005 Benjamin Eriksson & Mattias Pierre.
 * 
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */
static int consoleFd = -1;
static struct termios old;

int MVPD_START_INPUT(void)
{
  struct termios new;

  if ((consoleFd = open("/dev/console", O_NONBLOCK)) < 0)
    fprintf(stderr, "Could not open /dev/console");
  	
  if (tcgetattr(consoleFd, &old) < 0)
    fprintf(stderr, "Could not save old termios");
	
  new = old;
	
  new.c_lflag    &= ~(ICANON | ECHO  | ISIG);
  new.c_iflag    &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON | BRKINT);
  new.c_cc[VMIN]  = 0;
  new.c_cc[VTIME] = 0;
	
  if (tcsetattr(consoleFd, TCSAFLUSH, &new) < 0)
    exit(0);

  if (ioctl(consoleFd, KDSKBMODE, K_MEDIUMRAW) < 0)
    exit(0);
  
  return 0;
}

void MVPD_STOP_INPUT(void)
{	
  if (tcsetattr(consoleFd, TCSAFLUSH, &old) < 0)
    fprintf(stderr, "Could not reset old termios");
	
  if (consoleFd > 2)
    if (close(consoleFd) < 0)
      fprintf(stderr, "Could not close console");
}


int MVPD_GET_KEY(void)
{	
  int c = 0;
	
  if (read(consoleFd, &c, 1) != 1)
    c = -1;
  
  return c;
}
