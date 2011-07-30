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
 * Compute the Sobel edges of the luminosity channel (ignore chroma)
 * Write to standard output
 *
 */

int main(int argc, char *argv[])
{
  /* how to display edge images */
  enum { ALL, HORIZONTAL, VERTICAL, COMBINED } pickOut = COMBINED; /* default */
  int    showHough = 0, transformHough = 0;
  size_t threshHough = 0;

  /* how much to right shift the squared edge magnitudes */
  size_t shift = 5;  /* default */
  int optVal;
  while ( (optVal = getopt(argc, argv, "axyztl:s:h")) != -1 )
  {
    char c = optVal;
    if (c == 'a')
    {
      pickOut = ALL;
    }
    else if (c == 'x')
    {
      pickOut = HORIZONTAL;
    }
    else if (c == 'y')
    {
      pickOut = VERTICAL;
    }
    else if (c == 'z')
    {
      pickOut = COMBINED;
    }
    else if (c == 't')
    {
      transformHough = 1;
    }
    else if (c == 'l')
    {
      showHough = 1;
      threshHough = atoi(optarg);
    }
    else if (c == 's')
    {
      shift = atoi(optarg);
    }
    else if (c == 'h')
    {
      printf("Usage:    "
             "cat input.ppm | "
             "%s [-a|-x|-y|-z] [-l number] [-s number] > "
             "output.ppm\n"
             "  how edges are output (default is -z)\n"
             "      -a show -y as red, -x as green, luma as blue\n"
             "      -x only show horizontal edges (from vertical kernel)\n"
             "      -y only show vertical edges (from horizontal kernel)\n"
             "      -z show combined edges\n"
             "  optionally overlay Hough lines (only for combined edges)\n"
             "      -l show Hough detected edges with threshold\n"
             "      -t do not output edge image but instead Hough transform\n"
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

  /*
   * Note: The "X" image uses the Sobel kernel that detects vertical edges.
   *       The "Y" image uses the Sobel kernel that detects horizontal edges.
   *
   * This is somewhat counter-intuitive but makes more sense from what the
   * image processing convolution is doing.
   *
   */

  /* compute edges */
  SobelEdges(&edgeXImg, &edgeYImg, &lumaImg);

  /* edge magnitude */
  IMAGE8MALLOC( aImg, width, height )
  IMAGE8MALLOC( bImg, width, height )
  IMAGE8MALLOC( cImg, width, height )
  IMAGEHOUGHMALLOC( houghImg, width , height )
  IMAGE8MALLOC( outH, houghImg.width, houghImg.height )
  IMAGE8MALLOC( outH2, houghImg.width, houghImg.height )
  memset( outH2.data, 0, houghImg.width * houghImg.height );
  if (pickOut == COMBINED)
  {
    EdgeImagesToSS(&aImg, &edgeXImg, &edgeYImg, shift);

    /* optionally overlay the Hough lines */
    if (showHough)
    {
      const uint8_t  *etimg = aImg.data;
      const uint16_t *dximg = edgeXImg.data;
      const uint16_t  *dyimg = edgeYImg.data;

      size_t x = 0, y = 0;

      UNROLL_LOOP( width * height,

          if (*etimg++)
          {
            HoughVoteLine(&houghImg,
                          x - (width >> 1),
                          y - (height >> 1),
                          *dyimg,
                          *dximg,
                          4);
          }

          ++dximg;
          ++dyimg;

          if (++x == width)
          {
            x = 0;
            ++y;
          }
      )

      if (transformHough)
      {
        ConvertIntegralFeatureImage(&outH, &houghImg, 0);
      }

      const size_t *himg = houghImg.data;
      size_t theta, radius;
      for (radius = 0; radius < houghImg.height; ++radius)
      {
        for (theta = 0; theta < 128; ++theta)
        {
          if (*himg++ > threshHough)
          {
            DrawHoughLine(&cImg, width >> 1, height >> 1, theta, radius, 0xff);

            if (transformHough)
            {
              outH2.data[ (radius << 7) + theta ] = 0xff;
            }
          }
        }
      }
    }
  }
  else
  {
    EdgeImagesToSS(&aImg, &edgeXImg, &edgeXImg, shift);
    EdgeImagesToSS(&bImg, &edgeYImg, &edgeYImg, shift);
  }

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  if (pickOut == COMBINED && showHough && transformHough)
  {
    WritePPMHead(stdOut, houghImg.width, houghImg.height, components);
  }
  else
  {
    WritePPMHead(stdOut, width, height, components);
  }

  if (pickOut == ALL)
  {
    WritePPM888(stdOut, &aImg, &bImg, &lumaImg);
  }
  else if (pickOut == HORIZONTAL)
  {
    WritePPM888(stdOut, &bImg, &bImg, &bImg);
  }
  else if (pickOut == VERTICAL)
  {
    WritePPM888(stdOut, &aImg, &aImg, &aImg);
  }
  else if (pickOut == COMBINED)
  {
    if (showHough)
    {
      if (transformHough)
      {
        WritePPM888(stdOut, &outH2, &outH2, &outH);
      }
      else
      {
        WritePPM888(stdOut, &cImg, &aImg, &aImg);
      }
    }
    else
    {
      WritePPM888(stdOut, &aImg, &aImg, &aImg);
    }
  }
  fclose(stdOut);

  /* free memory */
  IMAGE8FREE( aImg )
  IMAGE8FREE( bImg )
  IMAGE8FREE( cImg )
  IMAGEHOUGHFREE( houghImg )
  IMAGE8FREE( outH )
  IMAGE8FREE( outH2 )
  IMAGE16FREE( edgeXImg )
  IMAGE16FREE( edgeYImg )
  IMAGE8FREE( lumaImg )
  IMAGE8FREE( chromaBImg )
  IMAGE8FREE( chromaRImg )
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )

  return 0;
}

