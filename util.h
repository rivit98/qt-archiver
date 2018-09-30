#pragma once

#include <sys/stat.h>
#include <string>

using std::string;

bool file_exists(string name);
string getExtension(const string & p);
