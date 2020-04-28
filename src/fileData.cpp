#include "fileData.h"


FileData::FileData() :
    offset(0),
    compressedSize(0){

}

FileData::FileData(const QString &path){
    QFileInfo fileInfo(path);
    if(fileInfo.exists()){
        this->path = path;
        this->filename = fileInfo.fileName();
        this->offset = 0;
        this->compressedSize = 0;
        normalizeFileName();
    }else{
        throw std::invalid_argument("File " + path.toStdString() + "does not exists");
    }
}

void FileData::normalizeFileName(){
    //just replace not ascii characters
    for(qint32 i = 0; i < filename.size(); i++){
        if(filename[i] > 127){
            filename[i] = '_';
        }
    }
}

QString FileData::getPath() const{
    return path;
}

QString FileData::getFilename() const{
    return filename;
}

quint32 FileData::getOffset() const{
    return offset;
}

quint32 FileData::getCompressedSize() const{
    return compressedSize;
}

void FileData::setCompressedSize(quint32 c){
    compressedSize = c;
}

void FileData::setOffset(quint32 o){
    offset = o;
}

qint64 FileData::getSizeofInternals() const{
    qint64 ret = 0;
    ret += filename.size();
    ret += sizeof(qint32);
    ret += sizeof(offset);
    ret += sizeof(compressedSize);
    return ret;
}

QByteArray FileData::getContents() const{
    QFile file(this->path);

    if(!file.exists()){
        return QByteArray();
    }

    file.open(QIODevice::ReadOnly);
    if(file.isOpen()){
        return QByteArray();
    }

    QByteArray contents = file.readAll();
    file.close();
    return contents;
}

bool FileData::saveToFile(const QByteArray& data, const QString& path) const{
    QFile outputFile(path + '/' + filename);
    if(outputFile.exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Yes", "File " + filename + " already exists!\nOverwrite?", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            return false;
        }
    }

    outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if(!outputFile.isOpen()){
        return false;
    }

    outputFile.write(data);
    outputFile.close();
    return true;
}

void FileData::readData(QDataStream& os){
    qint32 fnameSize;
    QByteArray buf;

    os >> fnameSize;
    this->filename.resize(fnameSize);
    buf.resize(fnameSize);

    os.readRawData(buf.data(), fnameSize);
    this->filename = QString(buf);

    os >> this->offset;
    os >> this->compressedSize;
}

bool FileData::operator==(const FileData& rhs){
    return this->filename == rhs.filename;
}

bool operator<(const FileData& lhs, const FileData& rhs){
    return lhs.getFilename() < rhs.getFilename();
}

uint qHash(const FileData& key){
    return qHash(key.getFilename());
}
