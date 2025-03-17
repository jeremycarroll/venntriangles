#include <stdio.h>
#include <stdlib.h>

#include "venn.h"

extern void printSolution(FILE* fp);

static char lastPrefix[128] = "";
static int solutionNumber = 0;

void writeSolution(char* prefix)
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
  printSolution(fp);
  for (COLOR a = 0; a < NCOLORS; a++) {
    findCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      fprintf(fp, "{%c:%d} ", color2char(a), i);
      if (corners[i][0] == NULL) {
        EDGE edge = &g_faces[NFACES - 1].edges[a];
        pLength = pathLength(edge, followEdgeBackwards(edge));
        fprintf(fp, "NULL/%d ", pLength);
      } else {
        pLength = pathLength(corners[i][0]->reversed, corners[i][1]);
        buffer[0] = buffer[1] = 0;
        fprintf(fp, "(%s => %s/%d) ", edge2str(buffer, corners[i][0]),
                edge2str(buffer, corners[i][1]), pLength);
      }
      numberOfVariations *= pLength;
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n\nVariations = %d\n", numberOfVariations);
  fclose(fp);
}
