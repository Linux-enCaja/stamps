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
 * Convert to YCbCr, show chroma channels equalized and luma edges
 * Write to standard output, Cb is blue, Cr is red, luma is green
 *
 */

int main(int argc, char *argv[])
{
  /* how much to right shift the squared edge magnitude */
  size_t shift = 5;  /* default */

  int optVal;
  while ( (optVal = getopt(argc, argv, "s:h")) != -1 )
  {
    char c = optVal;
    if (c == 's')
    {
      shift = atoi(optarg);
    }
    else if (c == 'h')
    {
      printf("Usage:    cat input.ppm | %s [-s number] > output.ppm\n"
             "  reduce edge magnitudes by a power of 2 (default is -s 5)\n"
             "      -s number of bits to right shift\n",
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

  /* edge images in horizontal and vertical directions */
  IMAGE16MALLOC( edgeXImg, width, height )
  IMAGE16MALLOC( edgeYImg, width, height )

  /* compute edges */
  SobelEdges(&edgeXImg, &edgeYImg, &lumaImg);

  /* edge magnitude */
  IMAGE8MALLOC( edgeImg, width, height )
  EdgeImagesToSS(&edgeImg, &edgeXImg, &edgeYImg, shift);

  /* equalize histograms of the chroma images so they are easier to see */
  HISTOGRAM8( chromaBHist )
  HISTOGRAM8( chromaRHist )
  ImageHistogram( &chromaBHist, &chromaBImg );
  ImageHistogram( &chromaRHist, &chromaRImg );
  EqualizeImage( &chromaBImg, &chromaBHist );
  EqualizeImage( &chromaRImg, &chromaRHist );

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  WritePPM888(stdOut, &chromaRImg, &edgeImg, &chromaBImg);
  fclose(stdOut);

  /* free memory */
  IMAGE8FREE( lumaImg )
  IMAGE8FREE( chromaBImg )
  IMAGE8FREE( chromaRImg )
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )
  IMAGE8FREE( edgeImg )

  return 0;
}

