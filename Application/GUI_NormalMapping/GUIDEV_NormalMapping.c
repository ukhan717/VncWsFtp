﻿/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEV_MoveAndFade.c
Purpose     : Routines for fading and moving memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include "GUIDEV_NormalMapping.h"
#if (GUI_SUPPORT_MEMDEV)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define LIGHT_THRESHOLD  0x40
#define LIGHT_MAX        0x30

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _CalcRect
*/
static void _CalcRect(GUI_RECT * pRect, GUI_MAPPING_CONTEXT * pContext, int xLight, int yLight) {
  int x;
  int y;
  int xSizeLight;
  int ySizeLight;

  xSizeLight = pContext->pbmLight->XSize;
  ySizeLight = pContext->pbmLight->YSize;
  x = xLight - (xSizeLight / 2);
  if (x < -(xSizeLight / 2)) {
    x = -(xSizeLight / 2);
  }
  y = yLight - (ySizeLight / 2);
  if (y < -(ySizeLight / 2)) {
    y = -(ySizeLight / 2);
  }
  pRect->x0 = (x < LIGHT_THRESHOLD) ? 0 : x - LIGHT_THRESHOLD;
  pRect->y0 = (y < LIGHT_THRESHOLD) ? 0 : y - LIGHT_THRESHOLD;
  pRect->x1 = ((x + xSizeLight + LIGHT_THRESHOLD) > pContext->xSize) ? pContext->xSize : x + xSizeLight + LIGHT_THRESHOLD;
  pRect->y1 = ((y + ySizeLight + LIGHT_THRESHOLD) > pContext->ySize) ? pContext->ySize : y + ySizeLight + LIGHT_THRESHOLD;
}

/*********************************************************************
*
*       _GetLightCoefficient
*/
static int _GetLightCoefficient(int NoLight, GUI_RECT Rect, int x, int y, const U16 * pNMap, const U8 * pLight, int xPos, int yPos, int xSize, int ySize) {
  I16 NormalX;
  I16 NormalY;
  I16 LightX;
  I16 LightY;
  int CLight;

  if (NoLight) {
    //
    // Without light the coeficient is always 0
    //
    CLight = 0;
  } else {
    //
    // If inside the light rectangle prcess shading
    //
    if ((x >= Rect.x0) && (x <= Rect.x1) && (y >= Rect.y0) && (y <= Rect.y1)) {
      //
      // Get x/y-data from the normal map
      //
      NormalX = (I8)((*pNMap >> 8) & 0xFF);
      NormalY = (I8) (*pNMap       & 0xFF);
      //
      // Calculate indices to access the light source
      //
      LightX = (x - xPos) + (xSize / 2) - NormalX;
      LightY = (y - yPos) + (ySize / 2) - NormalY;
      //
      // Check that indices are within the range
      //
      if ((LightX < 0)           || (LightY < 0)                  ||
          (LightX >= xSize) || (LightY >= ySize)) {
        CLight = 0;
      } else {
        //
        // Get the light coefficient
        //
        CLight = pLight[LightY * xSize + LightX];
      }
    } else {
      //
      // If outside light rectangle coeficient is 0
      //
      CLight = 0;
    }
  }
  return CLight;
}

/*********************************************************************
*
*       ShadeImage16
*/
static int ShadeImage16(GUI_MAPPING_CONTEXT * pContext, int xPosLight, int yPosLight) {
  U16      * pBmData;
  int        Offset;
  U8         CLight;
  U8         r, g, b;
  U16        Color;
  U16      * pNMap;
  U16      * pData;
  const U8 * pLight;
  U32        LightHalf;
  int        y;
  int        x;
  int        xSizeLight;
  int        ySizeLight;
  GUI_RECT   Rect;
  int        NoLight;

  //
  // Set up variables
  //
  pData = (U16 *)GUI_MEMDEV_GetDataPtr(pContext->hMemDest);
  if (pData == NULL) {
    return 1;
  }
  if (pContext->pBmImage) {
    pBmData = (U16 *)pContext->pBmImage->pData;
  } else {
    pBmData = (U16 *)GUI_MEMDEV_GetDataPtr(pContext->hMemImage);
    if (pBmData == NULL) {
      return 1;
    }
  }
  if (pContext->pbmLight) {
    pLight     = (const U8 *)pContext->pbmLight->pData;
    xSizeLight = pContext->pbmLight->XSize;
    ySizeLight = pContext->pbmLight->YSize;
    NoLight    = 0;
  } else {
    pLight     = NULL;
    xSizeLight = 0;
    ySizeLight = 0;
    NoLight    = 1;
  }
  LightHalf = LIGHT_MAX >> 1;
  Offset    = 0;
  //
  // Wihtout light the rectangle has the size of the image
  //
  if (NoLight) {
    Rect.x0 = 0;
    Rect.y0 = 0;
    Rect.x1 = pContext->xSize;
    Rect.y1 = pContext->ySize;
  } else {
    //
    // With light only the light area needs to be touched
    //
    _CalcRect(&Rect, pContext, xPosLight, yPosLight);
  }
  //
  // Itterate over the image
  //
  for (y = 0; y < pContext->ySize; y++) {
    for (x = 0; x < pContext->xSize; x += 1, Offset += 1) {
      //
      // Get pixel data from the normal map and the image to be shaded
      //
      pNMap = ((U16 *)pContext->pNMap) + Offset;
      Color = *(pBmData + Offset);
      CLight = _GetLightCoefficient(NoLight, Rect, x, y, pNMap, pLight, xPosLight, yPosLight, xSizeLight, ySizeLight);
      //
      // Grap the colors from the current pixel
      //
      r = (Color & 0xF800) >> 11;
      g = (Color & 0x07E0) >> 5;
      b =  Color & 0x001F;
      //
      // Apply coefficient
      //
      if (CLight == 0) {
        //
        // No light, darken pixel
        //
        r = (r >> 2);
        g = (g >> 2);
        b = (b >> 2);
      } else if (CLight <= LightHalf) {
        //
        // Apply a bit of light to pixels which are outside half of hte light radius
        //
        r = (r >> 2) + ((r - (r >> 2)) * CLight) / LightHalf;
        g = (g >> 2) + ((g - (g >> 2)) * CLight) / LightHalf;
        b = (b >> 2) + ((b - (b >> 2)) * CLight) / LightHalf;
      } else {
        //
        // Apply light to those within range
        //
        r = r + (((0x1F ^ r) * (CLight - LightHalf)) / LightHalf);
        g = g + (((0x3F ^ g) * (CLight - LightHalf)) / LightHalf);
        b = b + (((0x1F ^ b) * (CLight - LightHalf)) / LightHalf);
      }
      //
      // Shift colors bytes into order
      //
      Color = (r << 11) | (g << 5) | b;
      //
      // And write the pixel
      //
      *(pData + y * pContext->xSize + x) = Color;
    }
  }
  return 0;
}

/*********************************************************************
*
*       ShadeImage32
*/
static int ShadeImage32(GUI_MAPPING_CONTEXT * pContext, int xPosLight, int yPosLight) {
  U32      * pBmData;
  int        Offset;
  U8         CLight;
  U8         r, g, b, a;
  U32        Color;
  U16      * pNMap;
  U32      * pData;
  const U8 * pLight;
  U32        LightHalf;
  int        y;
  int        x;
  int        xSizeLight;
  int        ySizeLight;
  GUI_RECT   Rect;
  int        NoLight;

  //
  // Set up variables
  //
  pData = (U32 *)GUI_MEMDEV_GetDataPtr(pContext->hMemDest);
  if (pData == NULL) {
    return 1;
  }
  if (pContext->pBmImage) {
    pBmData = (U32 *)pContext->pBmImage->pData;
  } else {
    pBmData = (U32 *)GUI_MEMDEV_GetDataPtr(pContext->hMemImage);
    if (pBmData == NULL) {
      return 1;
    }
  }
  if (pContext->pbmLight) {
    pLight     = (const U8 *)pContext->pbmLight->pData;
    xSizeLight = pContext->pbmLight->XSize;
    ySizeLight = pContext->pbmLight->YSize;
    NoLight    = 0;
  } else {
    pLight     = NULL;
    xSizeLight = 0;
    ySizeLight = 0;
    NoLight    = 1;
  }
  LightHalf = LIGHT_MAX >> 1;
  Offset    = 0;
  //
  // Wihtout light the rectangle has the size of the image
  //
  if (NoLight) {
    Rect.x0 = 0;
    Rect.y0 = 0;
    Rect.x1 = pContext->xSize;
    Rect.y1 = pContext->ySize;
  } else {
    //
    // With light only the light area needs to be touched
    //
    _CalcRect(&Rect, pContext, xPosLight, yPosLight);
  }
  //
  // Itterate over the image
  //
  for (y = Rect.y0; y < Rect.y1; y++) {
    for (x = Rect.x0; x < Rect.x1; x += 1) {
      //
      // Get pixel data from the normal map and the image to be shaded
      //
      Offset = y * pContext->xSize + x;
      pNMap = ((U16 *)pContext->pNMap) + Offset;
      Color = *(pBmData + Offset);
      CLight = _GetLightCoefficient(NoLight, Rect, x, y, pNMap, pLight, xPosLight, yPosLight, xSizeLight, ySizeLight);
      //
      // Grap the colors from the current pixel
      //
      a = (Color & 0xFF000000) >> 24;
      r = (Color & 0x00FF0000) >> 16;
      g = (Color & 0x0000FF00) >> 8;
      b =  Color & 0x000000FF;
      //
      // Apply coefficient
      //
      if (CLight == 0) {
        //
        // No light, darken pixel
        //
        r = (r >> 2);
        g = (g >> 2);
        b = (b >> 2);
      } else if (CLight <= LightHalf) {
        //
        // Apply a bit of light to pixels which are outside half of hte light radius
        //
        r = (r >> 2) + ((r - (r >> 2)) * CLight) / LightHalf;
        g = (g >> 2) + ((g - (g >> 2)) * CLight) / LightHalf;
        b = (b >> 2) + ((b - (b >> 2)) * CLight) / LightHalf;
      } else {
        //
        // Apply light to those within range
        //
        r = r + (((0xFF ^ r) * (CLight - LightHalf)) / LightHalf);
        g = g + (((0xFF ^ g) * (CLight - LightHalf)) / LightHalf);
        b = b + (((0xFF ^ b) * (CLight - LightHalf)) / LightHalf);
      }
      //
      // Shift colors bytes into order
      //
      Color = (a << 24) | (r << 16) | (g << 8) | b;
      //
      // And write the pixel
      //
      *(pData + Offset) = Color;
    }
  }
  return 0;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_NormalMapping
*/
int GUI_MEMDEV_NormalMapping(GUI_MAPPING_CONTEXT * pContext, int xPosLight, int yPosLight) {
  switch (GUI_MEMDEV_GetBitsPerPixel(pContext->hMemDest)) {
  case 16:
    return ShadeImage16(pContext, xPosLight, yPosLight);
  case 32:
    return ShadeImage32(pContext, xPosLight, yPosLight);
  }
  return 1;
}

#else

void GUI_MEMDEV_NormalMapping(void);
void GUI_MEMDEV_NormalMapping(void) {} /* avoid empty object files */

#endif

/*************************** end of file ****************************/


