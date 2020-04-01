#pragma once

#include "lz4_win32_v1_9_2/include/lz4.h"
#include "filedata.h"
#include <QBuffer>

class Compressor
{
private:
    static constexpr int LZ4_BLOCKSIZE = 1024 * 1024;
public:
    Compressor() = default;
    QByteArray compress(const FileData& f) const;
    QByteArray uncompress(QByteArray& compressed) const;
};
