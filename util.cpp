#include "util.h"
#include <iostream>

bool file_exists( string name)
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

string getExtension(const string & p)
{
	return p.substr(p.find(".")+1);
}
