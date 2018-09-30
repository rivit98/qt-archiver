#pragma once

#include <string>
#include "util.h"
#include <fstream>

class FileData
{
private:
    std::string filename;
	char * data;
    unsigned int dataLen;
    unsigned int offset;
    int uncompressedLen;
public:
	FileData();
    FileData(std::string path);
    FileData(const FileData & f);
	~FileData();
	char * getDataPointer() const {return data;}
    unsigned int getOffset() const {return offset;}
    unsigned int getDataLength() const {return dataLen;}
    unsigned int getRealDataLen() const {return uncompressedLen;}
    const std::string getFilename() const {return filename;}
    void setOffset(unsigned int o) {offset = o;}
	char * allocMemory();
	FileData & operator=(const FileData & f);
	int getSizeOfAllDatas() const;
    bool saveFile(const std::string& folder) const;
    friend std::fstream & operator<< ( std::fstream & fs, const FileData & d );
    friend std::fstream & operator>> ( std::fstream & fs, FileData & d );
};
