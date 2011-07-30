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
 * Read in a binary RGB PPM (either 8 bit grayscale or 24 bit color) from
 * standard input
 * Write the image as a JPEG to standard output (either 8 bit grayscale or 24
 * bit color)
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
        printf("Usage:    cat input.ppm | %s > output.jpg\n", argv[0]);
        return 0;  /* exit */
    }
  }

  /* standard input */
  FILE *stdIn = fdopen(0, "r");
  READBUFFER( stdInBuf, 256 )

  size_t width, height, components;
  ReadPPMHead(&width, &height, &components, stdIn, &stdInBuf);

  if (components == 1)
  {
    /* one grayscale image */
    IMAGE8MALLOC( grayImg, width, height )

    /* read in the grayscale image */
    ReadPPM8(&grayImg, stdIn, &stdInBuf);

    /* libjpeg compressor stuff */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    /* write the image as an 8 bit grayscale JPEG */
    FILE *stdOut = fdopen(1, "w");

    WriteJPEGHead(stdOut, &cinfo, &jerr, width, height, components, JCS_GRAYSCALE, JCS_UNKNOWN);

    WriteJPEG8(stdOut, &cinfo, &grayImg);
    fclose(stdOut);

    /* free image memory */
    IMAGE8FREE( grayImg )
  }
  else
  {
    /* three images for red, green and blue */
    IMAGE8MALLOC( redImg, width, height )
    IMAGE8MALLOC( greenImg, width, height )
    IMAGE8MALLOC( blueImg, width, height )

    /* read in the RGB PPM image */
    ReadPPM888(&redImg, &greenImg, &blueImg, stdIn, &stdInBuf);

    /* libjpeg compressor stuff */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    /* write the image as a 24 bit color JPEG */
    FILE *stdOut = fdopen(1, "w");

    WriteJPEGHead(stdOut, &cinfo, &jerr, width, height, components, JCS_RGB, JCS_UNKNOWN);

    WriteJPEG888(stdOut, &cinfo, &redImg, &greenImg, &blueImg);
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

