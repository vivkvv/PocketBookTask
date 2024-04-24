#ifndef POCKET_BOOK_CODEC_LIB_H
#define POCKET_BOOK_CODEC_LIB_H

#include <memory>
#include <string>
#include <vector>
#include <atomic>

struct RawImageData {
    uint32_t width;
    uint32_t height;
    std::unique_ptr<unsigned char[]> data;
};

struct EncodedData {
    using dimension_type = uint32_t;

    unsigned char formatIdentifier[2];
    dimension_type width;
    dimension_type height;
    uint64_t dataSize;
    std::unique_ptr<unsigned char[]> data;
};

enum class PocketBookCodecLibErrorCode {
    ecOk = 0,
    ecLowMemory = 1,
    ecCantCreateOutputFile = 2,
    ecOutOfRange = 3,
    ecExceptionOccured = 4,
    ecCancel = 5,
    ecBadFormat = 6
};

class PocketDecoder {

public:
    PocketBookCodecLibErrorCode encode(
        std::unique_ptr<RawImageData> rawData,
        const std::string fileName) noexcept;

    PocketBookCodecLibErrorCode decode(
        std::unique_ptr<EncodedData> encodedData,
        const std::string fileName) noexcept;

    void setCancelFlag(bool flag);

    void cancel();

private:
    std::atomic<bool> m_cancel = false;

    PocketBookCodecLibErrorCode innerEncodeData(
        std::unique_ptr<RawImageData> rawData, const std::string fileName);

    PocketBookCodecLibErrorCode innerDecodeData(
        std::unique_ptr<EncodedData> encodedData, const std::string fileName);

    PocketBookCodecLibErrorCode writeEncodedData(
        const EncodedData& encodedData,
        std::vector<bool>& boolData,
        const std::string fileName);

    bool getBitByOffset(std::unique_ptr<EncodedData>& encodedData,
                        const uint64_t& dataOffset);

    bool getNextBit(std::unique_ptr<EncodedData>& encodedData,
                    uint64_t& dataOffset);

    std::vector<unsigned char> getNextFewBytes(
        uint8_t numBytes,
        const std::unique_ptr<EncodedData>& encodedData,
        uint64_t& dataOffset);

    bool allTheSame(const unsigned char* const begin,
                    const unsigned char* const end,
                    unsigned char PIXEL_COLOR);
};

#endif // POCKET_BOOK_CODEC_LIB_H
