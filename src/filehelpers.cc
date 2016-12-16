/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#include "filehelpers.h"

/*
 * DISCLAIMER:
 * This file is highly non-portable and probably works only on POSIX / *nix
 * systems.
 */
bool FileHelpers::file_exists(std::string filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}
int FileHelpers::getFileModificationTime(std::string filename) {
  struct stat buffer ;
  stat(filename.c_str(), &buffer);
  return buffer.st_mtime;
}
