#pragma once

#include <QVector>
#include <QMap>
#include <algorithm>

#include "lzma4_compressor.h"
#include "fileData.h"

class Archive
{
public:
    static constexpr const char *archive_extension = ".riv";
    static constexpr const quint16 magic = 0xDEAD;

private:
    QVector<FileData> fileList;
    QString path;
    QSharedPointer<Compressor> compressor;
    bool newFile;
    bool modified;

public:
    Archive();
    Archive(QString& path);
    ~Archive() = default;

    bool isNewArchive() const;
    bool isModified() const;
    void resetModified(bool m = false);
    bool isEmpty() const;
    bool isInArchive(const QString& filename) const;
    void setPath(const QString& p);
    qint32 getNumOfFiles() const;
    const QString& getPath() const;

    bool addFile(const QString& path);
    bool save();
    bool unpackFile(const QString& filename, const QString& dir) const;
    quint32 unpackAll(const QString& dir) const;
    const QVector<FileData>& getFiles() const;
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
