/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"

/**
 * Program entry point that delegates to the main implementation function.
 * This file is excluded from unity tests, which define their own main
 * functions.
 */
int main(int argc, char *argv[])
{
  return realMain0(argc, argv);
}
