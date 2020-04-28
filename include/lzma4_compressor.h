#pragma once

#include <QBuffer>

#include "lz4_win32_v1_9_2/lz4.h"
#include "fileData.h"
#include "compressor.h"

class LZMA4_Compressor : public Compressor
{
private:
    static constexpr qint32 LZ4_BLOCKSIZE = 1024 * 1024;

public:
    LZMA4_Compressor() = default;
    virtual ~LZMA4_Compressor() override = default;
    virtual QByteArray compress(const FileData& f) const override;
    virtual QByteArray uncompress(QByteArray& compressed) const override;
};
