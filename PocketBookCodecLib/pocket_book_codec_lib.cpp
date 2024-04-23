#include "pocket_book_codec_lib.h"

#include <algorithm>
#include <fstream>

#include <cstring>
#include <fstream>
#include <iostream>

#include <QtGui/QImage.h>

class OperationCanceledException : public std::runtime_error {
public:
    OperationCanceledException(const std::string& msg = "Operation was canceled")
        : std::runtime_error(msg) {}
};

enum class SequenceType {
    stAllWhite = 0,
    stAllBlack = 1,
    stDifferent = 2
};

const char* BMP_FORMAT = "BMP";
const unsigned char WHITE_PIXEL = 255;
const unsigned char BLACK_PIXEL = 0;
const int PIXELS_BLOCK_SIZE = 4;
const EncodedData::dimension_type DT_PIXELS_BLOCK_SIZE  = 4;

void PocketDecoder::setCancelFlag(bool flag) {
    m_cancel.store(flag);
}

void PocketDecoder::cancel() {
    m_cancel.store(true);
}

PocketBookCodecLibErrorCode PocketDecoder::encode(
    std::unique_ptr<RawImageData> rawData, const std::string fileName) noexcept {
    try{
        return innerEncodeData(std::move(rawData), fileName);
    }
    catch(OperationCanceledException&){
        return PocketBookCodecLibErrorCode::ecCancel;
    }
    catch(std::out_of_range&){
        return PocketBookCodecLibErrorCode::ecOutOfRange;
    }
    catch(...) {
        return PocketBookCodecLibErrorCode::ecExceptionOccured;
    }
}

PocketBookCodecLibErrorCode PocketDecoder::innerEncodeData(
    std::unique_ptr<RawImageData> rawData, const std::string fileName) {

    EncodedData encodedData {};
    encodedData.formatIdentifier[0] = 'B';
    encodedData.formatIdentifier[1] = 'A';
    encodedData.width = rawData->width;
    encodedData.height = rawData->height;
    encodedData.rowsIndex = std::make_unique<uint32_t[]>(encodedData.height);

    std::vector<bool> boolData;

    for (int row = 0; row < rawData->height; ++row){

        if(m_cancel.load()){
            throw OperationCanceledException();
        }

        unsigned char* sourceRow = rawData->data.get() + (row * rawData->width);

        bool allWhite = allTheSame(sourceRow, sourceRow + rawData->width, WHITE_PIXEL);

        encodedData.rowsIndex[row] = allWhite ? 1 : 0;

        if(allWhite) {
            continue;
        };

        for(int col = 0; col < rawData->width; col += PIXELS_BLOCK_SIZE){
            int rest = rawData->width - col;
            SequenceType seqType = {SequenceType::stDifferent};

            // find sequence type - white (0), black (10), different (11)
            if (rest >= PIXELS_BLOCK_SIZE) {
                bool whiteSeq = allTheSame(
                    sourceRow + col,
                    sourceRow + col + PIXELS_BLOCK_SIZE,
                    WHITE_PIXEL);

                if(whiteSeq) {
                    seqType = SequenceType::stAllWhite;
                }
                else {
                    bool blackSeq = allTheSame(
                        sourceRow + col,
                        sourceRow + col + PIXELS_BLOCK_SIZE,
                        BLACK_PIXEL);

                    if(blackSeq) {
                        seqType = SequenceType::stAllBlack;
                    }
                }
            }

            // apply sequence type coding
            switch(seqType) {
            case SequenceType::stAllWhite:
                boolData.push_back(0);
                break;
            case SequenceType::stAllBlack:
                boolData.push_back(1);
                boolData.push_back(0);
                break;
            default:
                boolData.push_back(1);
                boolData.push_back(1);

                for (size_t i = 0; i < std::min(rest, PIXELS_BLOCK_SIZE); ++i){
                    unsigned char byte = *(sourceRow + col + i);
                    for(int j = 0; j < 8; ++j){
                        boolData.push_back((byte & (1 << j)) ? 1 : 0);
                    }
                }
            }
        }
    }

    return writeEncodedData(encodedData, boolData, fileName);
}

bool PocketDecoder::allTheSame(const unsigned char* const begin,
                               const unsigned char* const end,
                               unsigned char PIXEL_COLOR) {

    return std::all_of(begin, end,
                       [PIXEL_COLOR](unsigned char pixel){
                           return pixel == PIXEL_COLOR;
                       }
                       );
}

PocketBookCodecLibErrorCode PocketDecoder::decode(
    std::unique_ptr<EncodedData> encodedData,
    const std::string fileName) noexcept {

    try {
        return innerDecodeData(std::move(encodedData), fileName);
    }
    catch(OperationCanceledException&){
        return PocketBookCodecLibErrorCode::ecCancel;
    }
    catch(std::out_of_range&){
        return PocketBookCodecLibErrorCode::ecOutOfRange;
    }
    catch(...) {
        return PocketBookCodecLibErrorCode::ecExceptionOccured;
    }
}

PocketBookCodecLibErrorCode PocketDecoder::innerDecodeData(
    std::unique_ptr<EncodedData> encodedData, const std::string fileName) {

    if (encodedData->formatIdentifier[0] != 'B' ||
        encodedData->formatIdentifier[1] != 'A') {
        return PocketBookCodecLibErrorCode::ecBadFormat;
    }

    QImage outputImage(encodedData->width, encodedData->height, QImage::Format_Grayscale8);

    outputImage.fill(Qt::white);

    uint64_t dataOffset = 0;

    for(size_t row = 0; row < encodedData->height; ++row){

        if(m_cancel.load()){
            throw OperationCanceledException();
        }

        if(encodedData->rowsIndex[row] == 1){
            continue;
        }

        EncodedData::dimension_type bytesInCurrentRow = 0;

        uchar* line = outputImage.scanLine(static_cast<int>(row));

        while (bytesInCurrentRow < encodedData->width) {
            bool currentBit = getNextBit(encodedData, dataOffset);

            if(!currentBit){
                std::memset(line + bytesInCurrentRow, 255, 4);
                bytesInCurrentRow += 4;
                continue;
            }

            currentBit = getNextBit(encodedData, dataOffset);
            if(!currentBit){
                std::memset(line + bytesInCurrentRow, 0, 4);
                bytesInCurrentRow += 4;
                continue;
            }

            EncodedData::dimension_type readBytes = std::min(
                DT_PIXELS_BLOCK_SIZE,
                encodedData->width - bytesInCurrentRow);

            std::vector<unsigned char> diffrentBytes = getNextFewBytes(
                readBytes,
                encodedData,
                dataOffset);

            std::memcpy(line + bytesInCurrentRow,
                        diffrentBytes.data(),
                        diffrentBytes.size());

            bytesInCurrentRow += readBytes;
        }
    }

    outputImage.save(fileName.c_str(), BMP_FORMAT);

    return PocketBookCodecLibErrorCode::ecOk;
}

PocketBookCodecLibErrorCode PocketDecoder::writeEncodedData(
        const EncodedData& encodedData,
        std::vector<bool>& boolData,
        const std::string fileName) {

    std::ofstream file(fileName, std::ios::out | std::ios::binary | std::ios::trunc);

    if(!file.is_open()) {
        return PocketBookCodecLibErrorCode::ecCantCreateOutputFile;
    }

    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    // header
    file.write(reinterpret_cast<const char*>(encodedData.formatIdentifier), sizeof(encodedData.formatIdentifier));
    file.write(reinterpret_cast<const char*>(&encodedData.width), sizeof(encodedData.width));
    file.write(reinterpret_cast<const char*>(&encodedData.height), sizeof(encodedData.height));

    // rows indexes
    file.write(reinterpret_cast<const char*>(encodedData.rowsIndex.get()), encodedData.height * sizeof(uint32_t));

    size_t bitsNeeded = 8 - (boolData.size() % 8);
    if (bitsNeeded != 8) {
        for (size_t i = 0; i < bitsNeeded; ++i){
            boolData.push_back(0);
        }
    }

    // convert bytes to bits
    std::vector<unsigned char> bytes(boolData.size() / 8, 0);
    for(size_t i = 0; i < boolData.size(); ++i) {
        if(boolData[i]) {
            bytes[i / 8] |= (1 << (7 - i % 8));
        }
    }

    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

    file.close();

    return PocketBookCodecLibErrorCode::ecOk;
}

bool PocketDecoder::getNextBit(std::unique_ptr<EncodedData>& encodedData,
                                    uint64_t& dataOffset){

    if(dataOffset / 8 >= encodedData->dataSize) {
        throw std::out_of_range("");
    }

    uint64_t byteIndex = dataOffset / 8;
    uint8_t bitIndex = 7 - (dataOffset % 8);

    bool bit = (encodedData->data[byteIndex] & (1 << bitIndex)) != 0;

    ++dataOffset;

    return bit;
}

std::vector<unsigned char> PocketDecoder::getNextFewBytes(
                uint8_t numBytes,
                const std::unique_ptr<EncodedData>& encodedData,
                uint64_t& dataOffset) {

    if(dataOffset + numBytes * 8 > encodedData->dataSize * 8){
        throw std::out_of_range("");
    }

    std::vector<unsigned char> result(numBytes, 0);

    for(int i = 0; i < numBytes * 8; ++i){

        uint64_t byteIndex = (dataOffset + i) / 8;
        uint64_t bitIndex = (dataOffset + i) % 8;

        unsigned char currentByte = encodedData->data[byteIndex];
        bool currentBit = (currentByte & (1 << (7 - bitIndex))) != 0;

        if(currentBit) {
            result[i / 8] |= (1 << (i % 8));
        }
    }

    dataOffset += numBytes * 8;

    return result;
}

