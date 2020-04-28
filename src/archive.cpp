#include "archive.h"

Archive::Archive() :
    compressor(new LZMA4_Compressor()),
    newFile(true),
    modified(false){

}

Archive::Archive(QString& path):
    Archive(){

    this->newFile = false;
    this->path = path;
    fileList.reserve(100);
    this->read();
}

void Archive::setPath(const QString& p){
    path = p;
}

void Archive::resetModified(bool m){
    modified = m;
}

const QString& Archive::getPath() const{
    return path;
}

bool Archive::isNewArchive() const{
    return newFile;
}

bool Archive::isModified() const{
    return modified;
}

qint32 Archive::getNumOfFiles() const{
    return fileList.size();
}

bool Archive::isEmpty() const{
    return getNumOfFiles() == 0;
}

const QVector<FileData>& Archive::getFiles() const{
    return fileList;
}

bool Archive::addFile(const QString& path){
    try{
        fileList.push_back(FileData(path));
        modified = true;
    }catch(std::invalid_argument){
        return false;
    }

    return true;
}

bool Archive::isInArchive(const QString& path) const{
    const FileData temp(path);
    QVectorIterator<FileData> iter(fileList);
    while(iter.hasNext()){
        if(iter.next().getFilename() == temp.getFilename()){
            return true;
        }
    }

    return false;
}

bool Archive::save(){
    if(isNewArchive()){
        return saveNew();
    }else{
        return saveExisting();
    }

    resetModified();
}

bool Archive::saveExisting(){
    qint32 headerSize = 0;
    for(const FileData& f : fileList){
        headerSize += f.getSizeofInternals();
    }

    QHash<QString, FileData> oldFiles;
    for(FileData& f : Archive(path).fileList){
        oldFiles.insert(f.getFilename(), f);
    }

    QFile oldArchiveFile(path);
    QFile tempArchive(path + ".tmp");
    oldArchiveFile.open(QIODevice::ReadOnly);
    tempArchive.open(QIODevice::WriteOnly);
    if(!oldArchiveFile.isOpen() || !tempArchive.isOpen()){
        QMessageBox::information(nullptr, "Error", "Can't save archive");
        return false;
    }

    quint32 startOffset = sizeof(Archive::magic) + sizeof(fileList.size());
    QDataStream out(&tempArchive);
    out.setByteOrder(QDataStream::LittleEndian);
    out << Archive::magic;
    out << fileList.size();
    out.device()->seek(startOffset + headerSize);

    QMutableVectorIterator<FileData> iter(fileList);

    while(iter.hasNext()){
        FileData& f = iter.next();
        QByteArray compressed;
        if(oldFiles.contains(f.getFilename())){ //this means that file data is compressed already and we need only copy it
            FileData& f_old = oldFiles[f.getFilename()];
            oldArchiveFile.seek(f_old.getOffset());
            compressed.reserve(f_old.getCompressedSize());
            compressed = oldArchiveFile.read(f_old.getCompressedSize());
        }else{
            compressed = compressor->compress(f);
        }
        f.setCompressedSize(compressed.size());
        f.setOffset(out.device()->pos());
        tempArchive.write(compressed.data(), compressed.size());
    }

    out.device()->seek(startOffset);
    iter.toFront();
    while(iter.hasNext()){
        FileData& f = iter.next();
        out << f.getFilename().size();
        tempArchive.write(f.getFilename().toLocal8Bit().constData(), f.getFilename().size());
        out << f.getOffset();
        out << f.getCompressedSize();
    }

    oldArchiveFile.close();
    tempArchive.close();
    QString origFname = QFile(path).fileName();
    oldArchiveFile.remove();
    tempArchive.rename(origFname);
    return true;
}

bool Archive::saveNew(){
    qint32 headerSize = 0;
    for(const FileData& f : fileList){
        headerSize += f.getSizeofInternals();
    }

    QFile archiveFile(path);
    archiveFile.open(QIODevice::WriteOnly);
    if(!archiveFile.isOpen()){
        QMessageBox::information(nullptr, "Error", "Can't save archive");
        return false;
    }

    quint32 startOffset = sizeof(magic) + sizeof(fileList.size());
    QDataStream out(&archiveFile);
    out.setByteOrder(QDataStream::LittleEndian);
    out << magic;
    out << fileList.size();
    out.device()->seek(startOffset + headerSize);

    QMutableVectorIterator<FileData> iter(fileList);

    while(iter.hasNext()){
        FileData& f = iter.next();
        QByteArray compressed = compressor->compress(f);
        f.setCompressedSize(compressed.size());
        f.setOffset(out.device()->pos());
        archiveFile.write(compressed.data(), compressed.size());
    }

    out.device()->seek(startOffset);
    iter.toFront();
    while(iter.hasNext()){
        FileData& f = iter.next();
        out << f.getFilename().size();
        archiveFile.write(f.getFilename().toLocal8Bit().constData(), f.getFilename().size());
        out << f.getOffset();
        out << f.getCompressedSize();
    }

    archiveFile.close();
    return true;
}

void Archive::read(){
    QFile archiveFile(path);
    archiveFile.open(QIODevice::ReadOnly);
    if(!archiveFile.isOpen()){
        QMessageBox::information(nullptr, "Error", "Can't open archive.");
        return;
    }

    QDataStream out(&archiveFile);
    out.setByteOrder(QDataStream::LittleEndian);

    quint16 r_magic;
    out >> r_magic;

    if(magic != r_magic)
    {
        QMessageBox::information(nullptr, "Error", "File " + archiveFile.fileName() + " is not a valid .riv archive.");
        return;
    }

    int r_filesNum;
    out >> r_filesNum;
    fileList.reserve(r_filesNum);

    for(int i = 0; i < r_filesNum; i++){
        FileData fd;
        fd.readData(out);
        fileList.push_back(fd);
    }

    archiveFile.close();
}

void Archive::removeFile(const QString& filename){
    const FileData* result = std::find_if(fileList.begin(), fileList.end(),
                                   [&filename](const FileData& f) {
                                        return f.getFilename() == filename;
                                    });
    if(result != fileList.end()){
        fileList.removeOne(*result);
        modified = true;
    }
}

bool Archive::unpackFile(const QString& filename, const QString& dir) const{
    const FileData* result = std::find_if(fileList.begin(), fileList.end(),
                                   [&filename](const FileData& f) {
                                        return f.getFilename() == filename;
                                    });
    if(result != fileList.end()){
        if(result->getCompressedSize() == 0){
            QMessageBox::information(nullptr, "Error", "Can't unpack " + filename + " (it is not packed)");
            return false;
        }

        QByteArray uncompressedData = getUncompressedFileData(*result);
        if(uncompressedData.size() == 0){
            return false;
        }
        return result->saveToFile(uncompressedData, dir);
    }

    return false;
}

QByteArray Archive::getUncompressedFileData(const FileData& f) const{
    QByteArray buffer;
    QFile archiveFile(path);
    archiveFile.open(QIODevice::ReadOnly);
    if(!archiveFile.isOpen()){
        QMessageBox::information(nullptr, "Error", "Can't read from current archive");
        return buffer;
    }

    archiveFile.seek(f.getOffset());
    buffer.reserve(f.getCompressedSize());
    buffer = archiveFile.read(f.getCompressedSize());
    buffer = compressor->uncompress(buffer);

    return buffer;
}

quint32 Archive::unpackAll(const QString& dir) const{
    quint32 cnt = 0;
    QByteArray uncompressedData;
    QVectorIterator<FileData> iter(fileList);
    while(iter.hasNext()){
        const FileData& fd = iter.next();
        if(fd.getCompressedSize() == 0){
            QMessageBox::information(nullptr, "Error", "Can't unpack " + fd.getFilename() + " (it is not packed)");
            continue;
        }

        uncompressedData.clear();
        uncompressedData = getUncompressedFileData(fd);
        if(uncompressedData.size() != 0){
            if(fd.saveToFile(uncompressedData, dir)){
                cnt++;
            }
        }
    }

    return cnt;
}
