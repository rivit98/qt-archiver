#pragma once

#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>

class FileData
{
    QString path;
    QString filename;
    quint32 offset;
    quint32 compressedSize;

public:
    FileData();
    FileData(const QString& path);
    ~FileData() = default;

    QString getPath() const;
    QString getFilename() const;
    quint32 getOffset() const;
    quint32 getCompressedSize() const;
    void setCompressedSize(quint32 c);
    void setOffset(quint32 o);

    QByteArray getContents() const;
    bool saveToFile(const QByteArray& data, const QString& path) const;
    qint64 getSizeofInternals() const;
    void readData(QDataStream& os);

    bool operator==(const FileData& rhs);
};

inline bool operator<(const FileData& lhs, const FileData& rhs);
inline uint qHash(const FileData& key);
