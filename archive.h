#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "fileData.h"


const short magic = 0xDEAD; // magic dla mojego formatu
const std::string ext = ".riv";
class Archive
{
	private:
        std::vector<FileData> listFiles;
        std::string apath;
        long long asize;
        bool modified;
	public:
        explicit Archive();
        Archive(const std::string & path); //load archive
		~Archive();
        void setPath(const std::string& p) { apath = p; }
        const std::string & getPath() const { return apath; }
        unsigned int getNumOfFiles() const { return listFiles.size(); }
        void add(const std::string & filePath);
		void save();
        bool unpack(const std::string & filename, const std::string& folder) const;
        const std::vector<FileData>& getFiles() const { return listFiles; }
        bool hasPath() const { return apath.size() != 0; }
        bool isInArchive(const std::string & name, long long size) const;
        long long size() const { return asize; }
        bool isModified() const { return modified; }
        void remove(const std::string& s);
        int unpackAll(const std::string& ) const;
};


/*
first two bytes - magic 0xDEAD
next four bytes are number of files in archive
next - header+data


*/
