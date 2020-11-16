#pragma once
#include <dirent.h>
#include <string>
#include <vector>

using std::vector;
using std::string;


vector<string> openDirectory(string path = ".") {
  DIR* dir;
  dirent* pdir;
  vector<string> files;

  dir = opendir(path.c_str());

  while (pdir = readdir(dir)) {
    string filename(pdir->d_name);
    string fileExtension("mtx");
    if (filename.find(fileExtension) == string::npos)
      continue;
    if ((filename.compare(".") != 0) && (filename.compare("..") != 0))
      files.push_back(filename);
  }
  return files;
}
