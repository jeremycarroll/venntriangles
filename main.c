#include "main.h"

#include "search.h"
#include "statistics.h"
#include "utils.h"
#include <sys/stat.h>

#include <getopt.h>
#include <stdlib.h>

static char *TargetFolder = NULL;

void dynamicSaveResult(void)
{
  char *buffer = getBuffer();
  snprintf(buffer, sizeof(buffer), "%s/%s", TargetFolder,
           dynamicFaceDegreeSignature());
  dynamicSolutionWrite(usingBuffer(buffer));
}

static void initializeOutputFolder()
{
  struct stat st = {0};

  if (stat(TargetFolder, &st) == -1) {
    // Directory does not exist, create it
    if (mkdir(TargetFolder, 0700) != 0) {
      perror("Failed to create directory");
      exit(EXIT_FAILURE);
    }
  } else {
    // Directory exists, check if it is writable
    if (!S_ISDIR(st.st_mode) || access(TargetFolder, W_OK) != 0) {
      fprintf(stderr, "Target folder exists but is not writable\n");
      exit(EXIT_FAILURE);
    }
  }
}

int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  while ((opt = getopt(argc, argv, "f:")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || TargetFolder == NULL) {
    fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
    return EXIT_FAILURE;
  }

  initializeOutputFolder();
  initializeStatisticLogging("/dev/stdout", 200, 10);
  dynamicSearchFull(dynamicSaveResult);
  statisticPrintFull();
  return 0;
}
