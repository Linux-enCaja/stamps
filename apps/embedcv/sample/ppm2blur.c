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
 * Blur this image using modified 3x3 box window (faster)
 * Write to standard output
 *
 */

int main(int argc, char *argv[])
{
  size_t repeat = 1;  /* default is blur only once */

  int optVal;
  while ( (optVal = getopt(argc, argv, "r:h")) != -1 )
  {
    char c = optVal;
    switch (c)
    {
      case ('r'):
        repeat = atoi(optarg);
        break;
      case ('h'):
        printf("Usage:    cat input.ppm | %s [-r num] > output.ppm\n"
               "  default is blur once (-r 1)\n"
               "      -r number of times to repeat blurring operation\n",
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

  /* blurred images, one for each channel */
  IMAGE8MALLOC( redBlur, width, height )
  IMAGE8MALLOC( greenBlur, width, height )
  IMAGE8MALLOC( blueBlur, width, height )

  Image8_t *ptrRedSrc   = &redImg;
  Image8_t *ptrGreenSrc = &greenImg;
  Image8_t *ptrBlueSrc  = &blueImg;

  Image8_t *ptrRedDest   = &redBlur;
  Image8_t *ptrGreenDest = &greenBlur;
  Image8_t *ptrBlueDest  = &blueBlur;

  Image8_t *tmp;

  while (repeat--)
  {
    BlurImage33Fast(ptrRedDest, ptrRedSrc);
    BlurImage33Fast(ptrGreenDest, ptrGreenSrc);
    BlurImage33Fast(ptrBlueDest, ptrBlueSrc);

    tmp = ptrRedDest;
    ptrRedDest = ptrRedSrc;
    ptrRedSrc = tmp;

    tmp = ptrGreenDest;
    ptrGreenDest = ptrGreenSrc;
    ptrGreenSrc = tmp;

    tmp = ptrBlueDest;
    ptrBlueDest = ptrBlueSrc;
    ptrBlueSrc = tmp;
  }

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  WritePPM888(stdOut, ptrRedSrc, ptrGreenSrc, ptrBlueSrc);
  fclose(stdOut);

  /* release image memory */
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )
  IMAGE8FREE( redBlur )
  IMAGE8FREE( greenBlur )
  IMAGE8FREE( blueBlur )

  return 0;
}

