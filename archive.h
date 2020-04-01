#pragma once

#include "compressor.h"
#include "filedata.h"

#include <QVector>
#include <QMap>
#include <algorithm>



class Archive
{
private:
    static constexpr quint16 magic = 0xDEAD;
    static const QString archive_extension;

    QVector<FileData> fileList;
    QString path;
    QSharedPointer<Compressor> compressor;
    bool newFile;
    bool modified;


public:
    Archive();
    Archive(QString& path);
    ~Archive() = default;

    qint32 getNumOfFiles() const;
    void setPath(const QString& p);
    void resetModified(bool m = false);
    const QString& getPath() const;
    bool addFile(const QString& path);
    bool isNewArchive() const;
    bool isModified() const;
    bool isInArchive(const QString& filename) const;
    bool save();
    bool unpackFile(const QString& filename, const QString& dir) const;
    quint32 unpackAll(const QString& dir) const;
    const QVector<FileData>& getData() const;
    void removeFile(const QString& filename);
    QByteArray getUncompressedFileData(const FileData& f) const;

private:
    void read();
    bool saveNew();
    bool saveExisting();
};

/*
    first two bytes - magic 0xDEAD
    next four bytes are number of files in archive
    next - header+data
*/
