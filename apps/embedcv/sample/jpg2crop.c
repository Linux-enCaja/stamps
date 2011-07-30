/*
 * EmbedCV - an embeddable computer vision library
 *
 * Copyright (C) 2006  Chris Jang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 *
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 * Email the author: cjang@ix.netcom.com
 *
 */



#include <stdio.h>
#include <unistd.h>

#include "embedcv.h"



/*
 * Read in a JPEG from standard input
 * Lossless crop out a subimage (does not compute IDCT)
 * Write the image as a JPEG to standard output
 *
 */

int main(int argc, char *argv[])
{
  size_t width, height, xoffset, yoffset;

  int optVal;
  while ( (optVal = getopt(argc, argv, "w:l:x:y:h")) != -1 )
  {
    char c = optVal;
    switch (c)
    {
      case ('w'):
        width = atoi(optarg);
        break;

      case ('l'):
        height = atoi(optarg);
        break;

      case ('x'):
        xoffset = atoi(optarg);
        break;

      case ('y'):
        yoffset = atoi(optarg);
        break;

      case ('h'):
        printf("Usage:    "
               "cat input.jpg | "
               "%s -w cropwidth -l cropheight -x xoffset -y yoffset > "
               "output.jpg\n", argv[0]);
        return 0;  /* exit */
    }
  }

  /* standard input */
  FILE *stdIn = fdopen(0, "r");

  /* standard output */
  FILE *stdOut = fdopen(1, "w");

  CropJPEG(stdOut, stdIn, width, height, xoffset, yoffset);

  fclose(stdIn);
  fclose(stdOut);

  return 0;
}

