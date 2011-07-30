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
 * Segment the image in luma or combined CbCr
 * Key patch comes from the center border area of one side
 * Write to standard output
 *
 */

int main(int argc, char *argv[])
{
  /* location of key patch */
  size_t optCol = 0, optRow = 0;  /* default is upper left corner */

  size_t patchSize  = 16;   /* default key is a square this size on a side */

  size_t threshold  = 1;      /* default segmentation threshold */
  int    thresholdIsSet = 0;  /* false */

  /* segment in luminosity or chroma */
  enum { LUMA, CHROMA } pickSeg = CHROMA;  /* default is chroma */
  int optVal;
  while ( (optVal = getopt(argc, argv, "x:y:p:t:lch")) != -1 )
  {
    char c = optVal;
    if (c == 'x')
    {
      optCol = atoi(optarg);
    }
    else if (c == 'y')
    {
      optRow = atoi(optarg);
    }
    else if (c == 'p')
    {
      patchSize = atoi(optarg);
    }
    else if (c == 'l')
    {
      pickSeg = LUMA;
    }
    else if (c == 'c')
    {
      pickSeg = CHROMA;
    }
    else if (c == 't')
    {
      threshold = atoi(optarg);
      thresholdIsSet = 1;  /* true */
    }
    else if (c == 'h')
    {
      printf("Usage:    "
             "cat input.ppm | "
             "%s [-x column] [-y row] [-p size] [-t threshold] [-l|-c] > "
             "output.ppm\n"
             "  location of key patch (default upper left corner -x 0 -y 0)\n"
             "      -x column\n"
             "      -y row\n"
             "  size of key patch (default pixel square -p 16)\n"
             "      -p number pixels per side "
             "(square size must be multiple of 8)\n"
             "  segmentation threshold radius (override default auto threshold)\n"
             "      -t distance from key patch\n"
             "  segment in luma or chroma (default is -c)\n"
             "      -l luminosity, the Y channel\n"
             "      -c chroma, the Cb and Cr channels\n",
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

  /* two images for luma and chroma CbCr packed */
  IMAGE8MALLOC( lumaImg, width, height )
  IMAGE16MALLOC( chromaImg, width, height )

  /* convert RGB to YCbCr */
  ConvertImageRGBtoYCbCrPacked(&lumaImg, &chromaImg,
                               &redImg, &greenImg, &blueImg);

  /* key patch comes from either luma or chroma images */
  IMAGE8MALLOC( lumaKey, patchSize, patchSize )
  IMAGE16MALLOC( chromaKey, patchSize, patchSize )
  /* key patch position in image */
  size_t patchColumn = (optCol < width - patchSize)
                           ? optCol
                           : (width - patchSize);
  size_t patchRow = (optRow < height - patchSize)
                        ? optRow
                        : (height - patchSize); 

  /*
   * crop out the key patch from the image in either luma or chroma
   * calculate histogram of the key patch sub image
   * calculate Otsu's segmentation threshold
   * construct the segmentation map lookup table
   *
   */
  HISTOGRAM8( yHist )
  HISTOGRAM8( cbHist )
  HISTOGRAM8( crHist )
  HISTOGRAM8( otsuYHist )
  HISTOGRAMCBCR( otsuCbCrHist )
  SEGMENTMAP( lumaMap )
  SEGMENTMAPCBCR( chromaMap )
  size_t otsuThreshold;
  if (pickSeg == LUMA)
  {
    CropImage(&lumaKey, &lumaImg, patchColumn, patchRow);
    ImageHistogram(&yHist, &lumaKey);
    uint8_t keyY = HistogramMedian(&yHist);
    ImageHistogramDist(&otsuYHist, &lumaImg, keyY);
    otsuThreshold = OtsuThreshold(&otsuYHist);
    SegmentMap(lumaMap,
               keyY,
               (thresholdIsSet) ? threshold : otsuThreshold,
               0x7f);
  }
  else  /* CHROMA */
  {
    CropImageW(&chromaKey, &chromaImg, patchColumn, patchRow);
    ImageHistogramCbCr(&cbHist, &crHist, &chromaKey);
    uint16_t keyCb = HistogramMedian(&cbHist);
    uint16_t keyCr = HistogramMedian(&crHist);
    uint16_t keyCbCr = (keyCr << 8) | keyCb;
    ImageHistogramCbCrDist(&otsuCbCrHist, &chromaImg, keyCbCr);
    otsuThreshold = OtsuThreshold(&otsuCbCrHist);
    SegmentMapCbCr(chromaMap,
                   keyCbCr,
                   (thresholdIsSet) ? threshold : otsuThreshold,
                   0x7f);
  }

  /* segment the image */
  IMAGE8MALLOC( segmentImg, width, height )
  if (pickSeg == LUMA)
  {
    SegmentImage(&segmentImg, &lumaImg, lumaMap);
  }
  else  /* CHROMA */
  {
    SegmentImageW(&segmentImg, &chromaImg, chromaMap);
  }

  /* display location of key patch box and Otsu's threshold */
  IMAGE8MALLOC( infoImg, width, height )
  memset(infoImg.data, 0, sizeof(uint8_t) * width * height);
  DrawImageBoundingBox(&infoImg,
                       patchColumn, patchRow, patchSize, patchSize,
                       0xff);
  char buffer[64];
  sprintf(buffer, "AUTO   %u", otsuThreshold);
  DrawImageString(&infoImg, buffer, 5, 5, 0xff, 0);
  if (thresholdIsSet)
  {
    sprintf(buffer, "MANUAL %u", threshold);
    DrawImageString(&infoImg, buffer, 5, 15, 0xff, 0);
  }

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  if (pickSeg == LUMA)
  {
    WritePPM888(stdOut, &infoImg, &segmentImg, &lumaImg);
  }
  else  /* CHROMA */
  {
    WritePPM888(stdOut, &infoImg, &segmentImg, &lumaImg);
  }
  fclose(stdOut);

  /* free memory */
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )
  IMAGE8FREE( lumaImg )
  IMAGE16FREE( chromaImg )
  IMAGE8FREE( lumaKey )
  IMAGE16FREE( chromaKey )
  IMAGE8FREE( segmentImg )
  IMAGE8FREE( infoImg )

  return 0;
}

