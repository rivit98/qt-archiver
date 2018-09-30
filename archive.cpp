#include "archive.h"
#include "fileData.h"
#include "util.h"
#include <iostream>
#include <string>
#include <fstream>
#include <QDebug>

Archive::Archive(){

}

Archive::Archive(const std::string & path) //load archive into memory
{
    modified = false;
	apath = path;
    std::fstream f;
    f.open(path, std::ios::binary | std::ios::in);
	if(!f.good())
	{
        std::cout << "Error while opening file [ " << path << " ]" << std::endl;
		return;
	}

    f.seekg(0, std::ios_base::end);
    asize = f.tellp();
    f.seekg(0, std::ios_base::beg);

	//check signature
	short val;
	f.read((char *)&val, sizeof(val));

	if(magic != val)
	{
        std::cout << "File [" << path << "] is not a valid archive format!" << std::endl;
		return;
	}

	int startOffset = sizeof(magic);
	int numberOfFiles = 0;
	f.read((char *)&numberOfFiles, sizeof(numberOfFiles));
	startOffset += sizeof(numberOfFiles);
    listFiles.resize(numberOfFiles);
    for(auto& iter : listFiles){
        f >> iter;
    }

    for(auto& iter : listFiles){
        f.seekg(iter.getOffset());
        f.read(iter.allocMemory(), iter.getDataLength());
    }

    f.close();
}

Archive::~Archive()
{
}

void Archive::add(const std::string & filePath)
{
    modified = true;
    listFiles.emplace_back(FileData(filePath));
}

void Archive::remove(const std::string& fileToDelete){
    modified = true;
    listFiles.erase(std::remove_if(listFiles.begin(), listFiles.end(),
                                [&](const FileData& file) { return file.getFilename() == fileToDelete; }),
                    listFiles.end());
}

int Archive::unpackAll(const std::string& folder) const{
    int ret = 0;
    for(std::vector<FileData>::const_iterator it = listFiles.begin(); it != listFiles.end(); it++){
        if((*it).saveFile(folder)){
            ret++;
        }
    }

    return ret;
}

bool Archive::unpack(const std::string& filename, const std::string& folder) const
{
    auto res = std::find_if(listFiles.begin(), listFiles.end(),
                            [&](const FileData& file) { return file.getFilename() == filename; });
    return ((*res).saveFile(folder));
}

void Archive::save()
{
    int vSize = listFiles.size();

    std::fstream f;
    f.open(apath, std::ios::binary | std::ios::out);
	f.write((char *)&magic, sizeof(magic)); //save magic
	f.write((char *)&vSize, sizeof(vSize));
	int startOffset = sizeof(magic) + sizeof(vSize);
	int headerSize = 0;

	for(int i = 0; i < vSize; i++)
	{
		headerSize += listFiles[i].getSizeOfAllDatas();
	}
	
	headerSize += startOffset; //magic, files num
	f.seekg(headerSize);

	for(int i = 0; i < vSize; i++)
	{
		listFiles[i].setOffset(f.tellp());
		f.write( listFiles[i].getDataPointer(), listFiles[i].getDataLength() );
	}
	f.seekg(startOffset);

	for(int i = 0; i < vSize; i++)
		f << listFiles[i];

	f.close();

    modified = false;
}

bool Archive::isInArchive(const std::string& name, long long size) const{
    for(auto& item : listFiles){
        if(item.getFilename() == name/* && item.getDataLength() == size*/){
            return true;
        }
    }

    return false;
}
