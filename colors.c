#include "venn.h"

bool memberOfColorSet(COLOR color, COLORSET colorSet)
{
  return (colorSet & (1u << color)) != 0;
}
