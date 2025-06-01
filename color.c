/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "color.h"

#include "utils.h"

#include <string.h>

/* Color operations */
int colorToChar(COLOR c)
{
  return 'a' + c;
}

/* Colorset operations */
char* colorSetToString(COLORSET colors)
{
  uint32_t i;
  char* buffer = getBuffer();
  char* p = buffer;
  *p++ = '|';
  for (i = 0; i < NCOLORS; i++) {
    if (colors & (1u << i)) {
      *p++ = 'a' + i;
    }
  }
  *p++ = '|';
  *p = '\0';
  return usingBuffer(buffer);
}

char* colorSetToBareString(COLORSET colors)
{
  char* colorsStr = colorSetToString(colors);
  colorsStr++;                           // delete first '|'
  colorsStr[strlen(colorsStr) - 1] = 0;  // delete last '|'
  return colorsStr;
}
