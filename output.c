

#include <stdlib.h>

#include "face.h"
/*
TODO: add graphml support.
*/

extern void dynamicSolutionPrint(FILE* fp);

static char lastPrefix[128] = "";
static int solutionNumber = 0;

void dynamicSolutionWrite(char* prefix)
{
  EDGE corners[3][2];
  char filename[1024];
  char buffer[1024];
  int numberOfVariations = 1;
  int pLength;
  FILE* fp;
  if (strcmp(prefix, lastPrefix) != 0) {
    strcpy(lastPrefix, prefix);
    solutionNumber = 1;
  }
  snprintf(filename, sizeof(filename), "%s-%2.2d.txt", prefix,
           solutionNumber++);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }
  dynamicSolutionPrint(fp);
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      fprintf(fp, "{%c:%d} ", colorToChar(a), i);
      if (corners[i][0] == NULL) {
        EDGE edge = &Faces[NFACES - 1].edges[a];
        pLength = edgePathLength(edge, edgeFollowBackwards(edge));
        fprintf(fp, "NULL/%d ", pLength);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1]);
        buffer[0] = buffer[1] = 0;
        fprintf(fp, "(%s => %s/%d) ", edgeToStr(buffer, corners[i][0]),
                edgeToStr(buffer, corners[i][1]), pLength);
      }
      numberOfVariations *= pLength;
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n\nVariations = %d\n", numberOfVariations);
  fclose(fp);
}
