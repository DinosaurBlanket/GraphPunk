
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
#include <fstream>

#include "readFile.hpp"

int readFile(const char *path, std::string &out) {
  std::ifstream fs(path);
  std::stringstream ss;
  ss << fs.rdbuf();
  if (fs.fail()) {cout << "Error reading file: " << path << endl;}
  out = ss.str();
  return out.length();
}