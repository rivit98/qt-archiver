#include "fileData.h"
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include <QMessageBox>
#include <QString>
#include "lz4.h"
#include <QDebug>

FileData::FileData()
{
}

FileData::~FileData()
{
    delete [] data;
}

FileData::FileData(const FileData & dt)
{
	offset = dt.offset;
	filename = dt.filename;
	dataLen = dt.dataLen;
    uncompressedLen = dt.uncompressedLen;
    data = new char[dataLen];
    std::memcpy(data, dt.data, dataLen);
}

FileData::FileData(std::string path)
{	
    unsigned int pos = path.rfind('/');
    if(pos == string::npos){
        filename = path;
    }
    else{
        filename = path.substr(pos+1);
    }
    offset = 0;

    std::fstream f;
    f.open(path, std::ios::binary | std::ios::in);
	if(!f.good())
	{
        std::cout << "Error while opening file [ " << path << " ]" << std::endl;
		return;
	}

    f.seekg(0, std::ios_base::end);
    uncompressedLen = f.tellp();
    f.seekg(0, std::ios_base::beg);

    char * uncompressedData = new char[uncompressedLen];
    f.read(uncompressedData, uncompressedLen);
	f.close();

    int lz_max =  LZ4_compressBound(uncompressedLen);

    data = new char[lz_max];
    dataLen = (unsigned int)LZ4_compress_default(uncompressedData, data, uncompressedLen, lz_max);

    if(dataLen == 0){ //LZ4_compress_default fails
        QMessageBox msgBox;
        msgBox.setText("This should not happened but happened\nThis file [" + QString::fromStdString(filename) + "] is corrupted!\n Don't use it :/");
        msgBox.exec();
        return;
    }

    delete[] uncompressedData;
}

bool FileData::saveFile(const std::string& folder) const
{
    std::string newPath = (folder + '/' + filename);
    if(file_exists(newPath))
	{
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Overwriting", "File " + QString::fromStdString(filename) + " exists! Overwrite?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return false;
	}

    char * uncompressedData = new char[uncompressedLen];
    int ret = LZ4_decompress_safe(data, uncompressedData, dataLen, uncompressedLen);

    if(ret < 0){ //LZ4_compress_default fails
        QMessageBox msgBox;
        msgBox.setText("This should not happened but happened\nThis file [" + QString::fromStdString(filename) + "] couldn't be decompressed");
        msgBox.exec();
        return false;
    }

    std::fstream f(newPath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    f.write(uncompressedData, uncompressedLen);
    f.close();

    delete[] uncompressedData;

	return true;
}

char * FileData::allocMemory()
{
	data = new char[dataLen];
	return data;
}

FileData & FileData::operator=(const FileData & dt)
{
	if(this == &dt)
		return *this;

	delete [] data;
	offset = dt.offset;
	filename = dt.filename;
	dataLen = dt.dataLen;
    uncompressedLen = dt.uncompressedLen;
	data = new char[dataLen];
    std::memcpy(data, dt.data, dataLen);

	return *this;
}

std::fstream & operator<<( std::fstream & fs, const FileData & d)
{
    int size = d.filename.size();
	fs.write((char*)&size, sizeof(size));
	fs.write(d.filename.c_str(), size);

	fs.write((char*)&d.dataLen, sizeof(d.dataLen));
	fs.write((char*)&d.offset, sizeof(d.offset));
    fs.write((char*)&d.uncompressedLen, sizeof(d.uncompressedLen));

	return fs;
}

int FileData::getSizeOfAllDatas() const
{
    int ret = 0;
    ret += filename.size();
    ret += sizeof(dataLen);
    ret += sizeof(offset);
    ret += sizeof(uncompressedLen);

    return ret;

}

std::fstream & operator>>( std::fstream & fs, FileData & d)
{
	int size = 0;

	fs.read((char *)&size, sizeof(size));
	d.filename.resize(size);
	fs.read(&d.filename[0], size);

	fs.read((char *)&d.dataLen, sizeof(d.dataLen));
	fs.read((char *)&d.offset, sizeof(d.offset));
    fs.read((char *)&d.uncompressedLen, sizeof(d.uncompressedLen));

    return fs;
}
