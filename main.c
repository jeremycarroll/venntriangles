#include "main.h"

#include "search.h"
#include "statistics.h"
#include <sys/stat.h>

#include <getopt.h>
#include <stdlib.h>

static char *targetFolder = NULL;

void dynamicSaveResult(void)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "%s/%s", targetFolder,
           dynamicFaceDegreeSignature());
  dynamicSolutionWrite(buffer);
}

static void initializeOutputFolder()
{
  struct stat st = {0};

  if (stat(targetFolder, &st) == -1) {
    // Directory does not exist, create it
    if (mkdir(targetFolder, 0700) != 0) {
      perror("Failed to create directory");
      exit(EXIT_FAILURE);
    }
  } else {
    // Directory exists, check if it is writable
    if (!S_ISDIR(st.st_mode) || access(targetFolder, W_OK) != 0) {
      fprintf(stderr, "Target folder exists but is not writable\n");
      exit(EXIT_FAILURE);
    }
  }
}

int dynamicMain0(int argc, char *argv[])
{
  int opt;
  targetFolder = NULL;
  while ((opt = getopt(argc, argv, "f:")) != -1) {
    switch (opt) {
      case 'f':
        targetFolder = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || targetFolder == NULL) {
    fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
    return EXIT_FAILURE;
  }

  initializeOutputFolder();
  initializeStatisticLogging("/dev/stdout", 200, 10);
  dynamicSearchFull(dynamicSaveResult);
  return 0;
}
