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
 * This is really a very special purpose step, unlike all of the other tools.
 *
 * It is for rudimentary object detection at the end of an image processing
 * chain (really a hack). Intersections between the red and green color channel
 * images after integral image transform features have been processed with
 * morphological dilation show overlapping up/down and left/right feature
 * blobs. Intersections of the red and green blob outlines appear as yellow.
 *
 * The intersection points are identified and placed in an array. All object
 * detection proceeds from this array. The images are no longer necessary.
 *
 * While the list is a good idea (maybe), the implementation is not. It is very
 * inefficient. This is a hack as the object detection infrastructure code has
 * not been written yet. Core library code will not be implemented this way.
 *
 * The value of this is to show how things can possibly work.
 *
 */


#define MAX_FEATURE_POINTS 2048


int main(int argc, char *argv[])
{
  size_t objThreshold = 8;   /* default of four points inside window */
  size_t objRadius    = 16;  /* default object radius is 8 pixels */

  int optVal;
  while ( (optVal = getopt(argc, argv, "t:r:h")) != -1 )
  {
    char c = optVal;
    switch (c)
    {
      case ('t'):
        objThreshold = atoi(optarg);
        break;

      case ('r'):
        objRadius = atoi(optarg);
        break;

      case ('h'):
        printf("Usage:    cat input.ppm | "
               "%s [-t threshold] [-r radius] > output.ppm\n"
               "  threshold for detected object (default is -t 8)\n"
               "      -t number of intersection points\n"
               "  object radius window (default is -r 16)\n"
               "      -r number of pixels\n",
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

  /* arrays for storing intersection points */
  size_t featureCol[ MAX_FEATURE_POINTS ];
  size_t featureRow[ MAX_FEATURE_POINTS ];
  size_t featureCount[ MAX_FEATURE_POINTS ];
  size_t numFeatures = 0;

  /* determine intersection points and put in array */
  size_t numPoints = 0;
  size_t colIdx, rowIdx, idx = 0;
  for (rowIdx = 0; rowIdx < height; rowIdx++)
  {
    for (colIdx = 0; colIdx < width; colIdx++)
    {
      /* look for all points with red and green at 0xff */
      if ( redImg.data[idx] == 0xff && greenImg.data[idx] == 0xff )
      {
        numPoints++;

        /* only store up to maximum number of feature points - beyond this,
         * object detection would become overwhelmed anyway - it means that
         * upstream in the image processing pipeline, that either the feature
         * scale should be changed or the thresholding more aggressive
         */
        if (numFeatures < MAX_FEATURE_POINTS)
        {
          featureCol[numFeatures] = colIdx;
          featureRow[numFeatures] = rowIdx;
          featureCount[numFeatures] = 0;
          numFeatures++;
        }
      }
      idx++;
    }
  }

  /* determine number of neighbor points (no sorting, very inefficient) */
  size_t ptCol, ptRow, colDist, rowDist, otherIdx;
  for (idx = 0; idx < numFeatures; idx++)
  {
    ptCol = featureCol[idx];
    ptRow = featureRow[idx];

    for (otherIdx = 0; otherIdx < numFeatures; otherIdx++)
    {
      colDist = UINTDIFF(ptCol, featureCol[otherIdx]);
      rowDist = UINTDIFF(ptRow, featureRow[otherIdx]);
      if ( (colDist + rowDist < objRadius) &&
           (UintSqrt(colDist * colDist + rowDist * rowDist) < objRadius) )
      {
        featureCount[idx]++;
      }
    }
  }

  /* find all points that exceed the object threshold */
  int featureActive[ MAX_FEATURE_POINTS ];
  size_t numThreshold = 0;
  for (idx = 0; idx < numFeatures; idx++)
  {
    if (featureCount[idx] > objThreshold)
    {
      featureCol[numThreshold] = featureCol[idx];
      featureRow[numThreshold] = featureRow[idx];
      featureActive[numThreshold] = 1;
      numThreshold++;
    }
  }

  /* for points exceeding the object threshold */
  size_t numObjects = 0;
  for (idx = 0; idx < numThreshold; idx++)
  {
    if (featureActive[idx])
    {
      numObjects++;

      ptCol = featureCol[idx];
      ptRow = featureRow[idx];

      /* mark all neighbor points as inactive */
      for (otherIdx = 0; otherIdx < numFeatures; otherIdx++)
      {
        colDist = UINTDIFF(ptCol, featureCol[otherIdx]);
        rowDist = UINTDIFF(ptRow, featureRow[otherIdx]);
        if ( (colDist + rowDist < objRadius) &&
             (UintSqrt(colDist * colDist + rowDist * rowDist) < objRadius) )
        {
          featureActive[otherIdx] = 0;
        }
      }

      /* draw a bounding box centered on the point */
      ptCol = (ptCol < (objRadius >> 1)) ? 0 : ptCol - (objRadius >> 1);
      ptRow = (ptRow < (objRadius >> 1)) ? 0 : ptRow - (objRadius >> 1);
      ptCol = (ptCol < width - objRadius) ? ptCol : width - objRadius;
      ptRow = (ptRow < width - objRadius) ? ptRow : width - objRadius;
      DrawImageBoundingBox(&redImg, ptCol, ptRow, objRadius, objRadius, 0xff);
      DrawImageBoundingBox(&greenImg, ptCol, ptRow, objRadius, objRadius, 0xff);
      DrawImageBoundingBox(&blueImg, ptCol, ptRow, objRadius, objRadius, 0xff);
    }
  }

  /* show some information about object detection */
  char buffer[64];
  sprintf(buffer, "TOTAL OBJ PTS  %u", numPoints);
  DrawImageString(&redImg, buffer, 5, 5, 0xff, -1);
  DrawImageString(&greenImg, buffer, 5, 5, 0xff, -1);
  DrawImageString(&blueImg, buffer, 5, 5, 0xff, -1);
  sprintf(buffer, "THRESH OBJ PTS %u", numThreshold);
  DrawImageString(&redImg, buffer, 5, 15, 0xff, -1);
  DrawImageString(&greenImg, buffer, 5, 15, 0xff, -1);
  DrawImageString(&blueImg, buffer, 5, 15, 0xff, -1);
  sprintf(buffer, "NUMBER OBJECTS %u", numObjects);
  DrawImageString(&redImg, buffer, 5, 25, 0xff, -1);
  DrawImageString(&greenImg, buffer, 5, 25, 0xff, -1);
  DrawImageString(&blueImg, buffer, 5, 25, 0xff, -1);

  /* write the image as a 24 bit RGB PPM */
  FILE *stdOut = fdopen(1, "w");
  WritePPMHead(stdOut, width, height, components);
  WritePPM888(stdOut, &redImg, &greenImg, &blueImg);
  fclose(stdOut);

  /* release image memory */
  IMAGE8FREE( redImg )
  IMAGE8FREE( greenImg )
  IMAGE8FREE( blueImg )

  return 0;
}

