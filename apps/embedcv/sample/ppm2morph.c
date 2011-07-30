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
 * Read a 24 bit binary RGB PPM image from standard input
 * Morphologically erode/dilate/open/close the image channel specified
 * (default is eroding all channels)
 * Write to standard output
 *
 */

int main(int argc, char *argv[])
{
  /* which color channel to erode */
  enum { ALLCOLORS, RED, GREEN, BLUE } pickImg = ALLCOLORS;  /* default */
  enum { ERODE, DILATE, OPEN, CLOSE } pickOp = ERODE;  /* default */

  size_t repeat = 1;  /* default number of times to perform operation */

  int optVal;
  while ( (optVal = getopt(argc, argv, "rgbd:e:o:c:h")) != -1 )
  {
    char c = optVal;
    switch (c)
    {
      case ('r'):
        pickImg = RED;
        break;
      case ('g'):
        pickImg = GREEN;
        break;
      case ('b'):
        pickImg = BLUE;
        break;
      case ('d'):
        pickOp = DILATE;
        repeat = atoi(optarg);
        break;
      case ('e'):
        pickOp = ERODE;
        repeat = atoi(optarg);
        break;
      case ('o'):
        pickOp = OPEN;
        repeat = atoi(optarg);
        break;
      case ('c'):
        pickOp = CLOSE;
        repeat = atoi(optarg);
        break;
      case ('h'):
        printf("Usage:    "
               "cat input.ppm | %s [-r|-g|-b] "
               "[-d num|-e num|-o num|-c num] > output.ppm\n"
               "  which color channel to process (default is all channels)\n"
               "      -r red image channel only\n"
               "      -g green image channel only\n"
               "      -b blue image channel only\n"
               "  morphological operation (default is -e 1)\n"
               "      -d number of times repeat dilation (expand region)\n"
               "      -e number of times repeat erosion (shrink region)\n"
               "      -o number of times repeat opening (remove small blobs)\n"
               "      -c number of times repeat closing (fill in holes)\n",
               argv[0]);
        return 0;  /* exit */
    }
  }

  /* stream I/O reading is buffered */
  FILE *stdIn = fdopen(0, "r");
  READBUFFER( stdInBuf, 256 )

  /* image dimensions */
  size_t width, height, components;

  /* read PPM header to know the image dimensions */
  ReadPPMHead(&width, &height, &components, stdIn, &stdInBuf);

  /* three images for RGB */
  IMAGE8MALLOC( redImg, width, height )
  IMAGE8MALLOC( greenImg, width, height )
  IMAGE8MALLOC( blueImg, width, height )

  /* read in the RGB PPM image */
  ReadPPM888(&redImg, &greenImg, &blueImg, stdIn, &stdInBuf);

  /* no need to read from stream anymore */
  fclose(stdIn);

  /* erode the specified image (default is all of them) */
  size_t num = repeat;
  switch (pickImg)
  {
    case (ALLCOLORS):
      if (pickOp == ERODE)
      {
        while (repeat--)
        {
          RegionErode55(&redImg, 0);
          RegionErode55(&greenImg, 0);
          RegionErode55(&blueImg, 0);
        }
      }
      else if (pickOp == DILATE)
      {
        while (repeat--)
        {
          RegionDilate55(&redImg, 0xff);
          RegionDilate55(&greenImg, 0xff);
          RegionDilate55(&blueImg, 0xff);
        }
      }
      else if (pickOp == OPEN)
      {
        while (num--)
        {
          RegionErode55(&redImg, 0);
          RegionErode55(&greenImg, 0);
          RegionErode55(&blueImg, 0);
        }
        while (repeat--)
        {
          RegionDilate55(&redImg, 0xff);
          RegionDilate55(&greenImg, 0xff);
          RegionDilate55(&blueImg, 0xff);
        }
      }
      else
      {
        while (num--)
        {
          RegionDilate55(&redImg, 0xff);
          RegionDilate55(&greenImg, 0xff);
          RegionDilate55(&blueImg, 0xff);
        }
        while (repeat--)
        {
          RegionErode55(&redImg, 0);
          RegionErode55(&greenImg, 0);
          RegionErode55(&blueImg, 0);
        }
      }
      break;

    case (RED):
      if (pickOp == ERODE)
      {
        while (repeat--)
        {
          RegionErode55(&redImg, 0);
        }
      }
      else if (pickOp == DILATE)
      {
        while (repeat--)
        {
          RegionDilate55(&redImg, 0xff);
        }
      }
      else if (pickOp == OPEN)
      {
        while (num--)
        {
          RegionErode55(&redImg, 0);
        }
        while (repeat--)
        {
          RegionDilate55(&redImg, 0xff);
        }
      }
      else  /* CLOSE */
      {
        while (num--)
        {
          RegionDilate55(&redImg, 0xff);
        }
        while (repeat--)
        {
          RegionErode55(&redImg, 0);
        }
      }
    break;

    case (GREEN):
      if (pickOp == ERODE)
      {
        while (repeat--)
        {
          RegionErode55(&greenImg, 0);
        }
      }
      else if (pickOp == DILATE)
      {
        while (repeat--)
        {
          RegionDilate55(&greenImg, 0xff);
        }
      }
      else if (pickOp == OPEN)
      {
        while (num--)
        {
          RegionErode55(&greenImg, 0);
        }
        while (repeat--)
        {
          RegionDilate55(&greenImg, 0xff);
        }
      }
      else  /* CLOSE */
      {
        while (num--)
        {
          RegionDilate55(&greenImg, 0xff);
        }
        while (repeat--)
        {
          RegionErode55(&greenImg, 0);
        }
      }
      break;

    case (BLUE):
      if (pickOp == ERODE)
      {
        while (repeat--)
        {
          RegionErode55(&blueImg, 0);
        }
      }
      else if (pickOp == DILATE)
      {
        while (repeat--)
        {
          RegionDilate55(&blueImg, 0xff);
        }
      }
      else if (pickOp == OPEN)
      {
        while (num--)
        {
          RegionErode55(&blueImg, 0);
        }
        while (repeat--)
        {
          RegionDilate55(&blueImg, 0xff);
        }
      }
      else  /* CLOSE */
      {
        while (num--)
        {
          RegionDilate55(&blueImg, 0xff);
        }
        while (repeat--)
        {
          RegionErode55(&blueImg, 0);
        }
      }
      break;
  }

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  WritePPM888(stdOut, &redImg, &greenImg, &blueImg);
  fclose(stdOut);

  /* free memory */
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )

  return 0;
}

