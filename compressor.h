#pragma once

#include <QBuffer>

#include "fileData.h"

class Compressor
{
public:
    Compressor() = default;
    virtual ~Compressor() = default;
    virtual QByteArray compress(const FileData& f) const = 0;
    virtual QByteArray uncompress(QByteArray& compressed) const = 0;
};
