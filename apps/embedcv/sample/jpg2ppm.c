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
 * Read in a JPEG from standard input (8 bit grayscale or 24 bit color)
 * Write the image as a PPM to standard output (8 bit grayscale or 24 bit color)
 *
 */

int main(int argc, char *argv[])
{
  int optVal;
  while ( (optVal = getopt(argc, argv, "h")) != -1 )
  {
    char c = optVal;
    switch (c)
    {
      case ('h'):
        printf("Usage:    cat input.jpg | %s > output.ppm\n", argv[0]);
        return 0;  /* exit */
    }
  }

  /* standard input */
  FILE *stdIn = fdopen(0, "r");

  /* libjpeg decompressor stuff */
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;

  /* image dimensions */
  size_t width, height, components;

  /* read JPEG header to know image dimensions */
  ReadJPEGHead(&width, &height, &components, stdIn, &cinfo, &jerr, JCS_UNKNOWN, 0);

  if (components == 1)
  {
    /* one image */
    IMAGE8MALLOC( grayImg, width, height )

    /* decompress JPEG into the image */
    ReadJPEG8(&grayImg, stdIn, &cinfo);

    /* write the image as an 8 bit grayscale PPM */
    FILE *stdOut = fdopen(1, "w");
    WritePPMHead(stdOut, width, height, components);
    WritePPM8(stdOut, &grayImg);
    fclose(stdOut);

    /* free image memory */
    IMAGE8FREE( grayImg )
  }
  else if (components == 3)
  {
    /* three images for red, green and blue */
    IMAGE8MALLOC( redImg, width, height )
    IMAGE8MALLOC( greenImg, width, height )
    IMAGE8MALLOC( blueImg, width, height )

    /* decompress JPEG into the RGB images */
    ReadJPEG888(&redImg, &greenImg, &blueImg, stdIn, &cinfo);

    /* write the image as a 24 bit RGB PPM */
    FILE *stdOut = fdopen(1, "w");
    WritePPMHead(stdOut, width, height, components);
    WritePPM888(stdOut, &redImg, &greenImg, &blueImg);
    fclose(stdOut);

    /* free image memory */
    IMAGE8FREE( redImg )
    IMAGE8FREE( greenImg )
    IMAGE8FREE( blueImg )
  }

  /* no need to read from stream anymore */
  fclose(stdIn);

  return 0;
}

