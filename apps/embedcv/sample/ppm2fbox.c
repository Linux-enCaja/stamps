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
 * Convert to YCbCr, we are only interested in the luma channel
 * Calculate integral image transform of luma image
 * Calculate images for up/down and left/right features
 * Write to standard output, red is left/right, green is up/down, blue is luma
 *
 */

int main(int argc, char *argv[])
{
  size_t shift   = 5;    /* default right shift of feature pixel values */
  size_t shiftoffset;    /* automatic scaling offset */
  size_t boxSize = 8;    /* default size of small dimension of feature boxes */

  int optVal;
  while ( (optVal = getopt(argc, argv, "p:s:a:h")) != -1 )
  {
    char c = optVal;
    if (c == 'p')
    {
      boxSize = atoi(optarg);
    }
    else if (c == 's')
    {
      shift = atoi(optarg);
    }
    else if (c == 'a')
    {
      shift       = 999;  /* impossible value indicates do automatic scaling */
      shiftoffset = atoi(optarg);
    }
    else if (c == 'h')
    {
      printf("Usage:    "
             "cat input.ppm | %s [-p size] [-s shift|-a offset] > output.ppm\n"
             "  size of feature boxes (default -p 8)\n"
             "      -p number pixels of box small dimension\n"
             "  reduce feature magnitudes by a power of 2 (default is -s 5)\n"
             "      -s number of bits to right shift\n"
             "      -a number bits below auto scaled maximum\n",
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

  /* three images for luma, chroma B and chroma R */
  IMAGE8MALLOC( lumaImg, width, height )
  IMAGE8MALLOC( chromaBImg, width, height )
  IMAGE8MALLOC( chromaRImg, width, height )

  /* convert RGB to YCbCr */
  ConvertImageRGBtoYCbCr(&lumaImg, &chromaBImg, &chromaRImg,
                         &redImg, &greenImg, &blueImg);

  /* equalize the luma image, this helps detect features (I think) */
  EQUALIZEIMG( lumaImg )

  /* calculate integral image transform */
  IMAGE32MALLOC( iiImg, width, height )
  IntegralImage(&iiImg, &lumaImg);

  /* dense feature images, probably never do this in practice */
  const size_t boxStep = 1;

  /* up/down feature image */
  const size_t updownBoxWidth  = boxSize << 1;
  const size_t updownBoxHeight = boxSize;
  const size_t updownImgWidth  = (width - updownBoxWidth) / boxStep;
  const size_t updownImgHeight = (height - (updownBoxHeight << 1)) / boxStep;
  IMAGE32MALLOC( updownImg, updownImgWidth, updownImgHeight )
  size_t updownMaxValue;
  IntegralFeatureUpDown(&updownImg, &updownMaxValue,
                        &iiImg,
                        updownBoxWidth,
                        updownBoxHeight,
                        boxStep,
                        boxStep);

  /* left/right feature image */
  const size_t leftrightBoxWidth  = boxSize;
  const size_t leftrightBoxHeight = boxSize << 1;
  const size_t leftrightImgWidth  = (width - (leftrightBoxWidth << 1))
                                        / boxStep;
  const size_t leftrightImgHeight = (height - leftrightBoxHeight) / boxStep;
  IMAGE32MALLOC( leftrightImg, leftrightImgWidth, leftrightImgHeight )
  size_t leftrightMaxValue;
  IntegralFeatureLeftRight(&leftrightImg, &leftrightMaxValue,
                           &iiImg,
                           leftrightBoxWidth,
                           leftrightBoxHeight,
                           boxStep,
                           boxStep);

  /* convert integral feature images to 8 bits */
  IMAGE8MALLOC( updown8Img, width, height )
  IMAGE8MALLOC( leftright8Img, width, height )
  if (shift != 999)
  {
    /* user specified shift factor */
    ConvertIntegralFeatureImage(&updown8Img, &updownImg, shift);
    ConvertIntegralFeatureImage(&leftright8Img, &leftrightImg, shift);
  }
  else
  {
    /* automatic */
    size_t updownShift = 0, leftrightShift = 0;

    while (updownMaxValue)
    {
      updownMaxValue >>= 1;
      updownShift++;
    }

    while (leftrightMaxValue)
    {
      leftrightMaxValue >>= 1;
      leftrightShift++;
    }

    ConvertIntegralFeatureImage(&updown8Img, &updownImg,
                                updownShift - shiftoffset);
    ConvertIntegralFeatureImage(&leftright8Img, &leftrightImg,
                                leftrightShift - shiftoffset);
  }

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  WritePPM888(stdOut, &leftright8Img, &updown8Img, &lumaImg);
  fclose(stdOut);

  /* free memory */
  IMAGE8FREE( lumaImg )
  IMAGE8FREE( chromaBImg )
  IMAGE8FREE( chromaRImg )
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )
  IMAGE32FREE( iiImg )
  IMAGE32FREE( updownImg )
  IMAGE32FREE( leftrightImg )
  IMAGE8FREE( updown8Img )
  IMAGE8FREE( leftright8Img )

  return 0;
}

