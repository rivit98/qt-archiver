#include "lzma4_compressor.h"

const qint32 LZMA4_Compressor::LZ4_BLOCKSIZE;

QByteArray LZMA4_Compressor::compress(const FileData& f) const
{
    QFile file(f.getPath());
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen()){
        QMessageBox::information(nullptr, "Error", "Can't open file: " + f.getPath());
        return QByteArray();
    }

    QByteArray uncompressed;
    uncompressed = file.readAll();
    const char *data = uncompressed.data();
    qint32 nbytes = uncompressed.size();

    // internal compress function
    auto compress = [&f](const char *src, int srclen, QByteArray &buffer) {
        const int bufsize = LZ4_compressBound(srclen);
        buffer.reserve(bufsize);

        if (srclen > 0){
            int rv = LZ4_compress_default(src, buffer.data(), srclen, bufsize);
            if (rv > 0) {
                buffer.resize(rv);
            }else{
                QMessageBox::information(nullptr, "Error", "LZ4 compression error: " + f.getPath());
                buffer.clear();
            }
        }else{
            buffer.resize(0);
        }
    };

    QByteArray ret; //TODO: this really should be splitted into chunks
    int rsvsize = LZ4_compressBound(nbytes);
    if (rsvsize < 1) {
        return ret;
    }

    ret.reserve(rsvsize);
    QDataStream dsout(&ret, QIODevice::WriteOnly);
    dsout.setByteOrder(QDataStream::LittleEndian);
    QByteArray buffer;
    int readlen = 0;

    while (readlen < nbytes) {
        int datalen = qMin(nbytes - readlen, LZMA4_Compressor::LZ4_BLOCKSIZE);
        compress(data + readlen, datalen, buffer);
        readlen += datalen;

        if (buffer.isEmpty()) {
            ret.clear();
            break;
        } else {
            dsout << (int)buffer.length();
            dsout.writeRawData(buffer.data(), buffer.length());
        }
    }

    return ret;
}

QByteArray LZMA4_Compressor::uncompress(QByteArray& compressed) const{
    char *data = compressed.data();
    int nbytes = compressed.size();
    QByteArray ret;
    QBuffer srcbuf;
    const int CompressBoundSize = LZ4_compressBound(LZ4_BLOCKSIZE);

    srcbuf.setData(data, nbytes);
    srcbuf.open(QIODevice::ReadOnly);
    QDataStream dsin(&srcbuf);
    dsin.setByteOrder(QDataStream::LittleEndian);

    QByteArray buffer;
    buffer.reserve(LZ4_BLOCKSIZE);

    int readlen = 0;
    while (readlen < nbytes) {
        int srclen;
        dsin >> srclen;
        readlen += sizeof(srclen);

        if (srclen <= 0 || srclen > CompressBoundSize) {
            QMessageBox::information(nullptr, "Error", "LZ4 uncompression error");
            ret.clear();
            break;
        }

        int rv = LZ4_decompress_safe(data + readlen, buffer.data(), srclen, LZ4_BLOCKSIZE);
        dsin.skipRawData(srclen);
        readlen += srclen;

        if (rv > 0) {
            buffer.resize(rv);
            ret += buffer;
        } else {
            QMessageBox::information(nullptr, "Error", "LZ4 uncompression error");
            ret.clear();
            break;
        }
    }
    return ret;
}
