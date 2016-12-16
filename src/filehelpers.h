/* 
 * Copyright (c) 2016 by Jakob Simeth
 * Licensed under MIT License. See LICENSE in the root directory.
 */
#ifndef __FILEHELPERS_H__
#define __FILEHELPERS_H__
#include <sys/stat.h>
#include <string>
namespace FileHelpers{
bool file_exists(std::string filename);
int getFileModificationTime(std::string filename);
}
#endif
