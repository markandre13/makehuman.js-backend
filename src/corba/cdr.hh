#pragma once

#include <bit>
#include <cctype>
#include <string>
#include <vector>

namespace CORBA {

// Common Data Representation

class CDREncoder {
        std::vector<char> _data;

    public:
        void ulong(uint32_t);
        const char *data() { return _data.data(); }
        size_t length() { return _data.size(); }
};

class CDRDecoder {
    public:
        const char *_data;
        size_t offset;
        size_t length;
        std::endian _endian;

        CDRDecoder() : _data(nullptr), offset(0), length(0) {}
        CDRDecoder(const char *data, size_t length, std::endian endian = (std::endian)0) : _data(data), offset(0), length(length), _endian(endian) {}
        CDRDecoder(CDREncoder &encoder) : _data(encoder.data()), offset(0), length(encoder.length()), _endian(std::endian::native) {}
        std::string_view toString() const { return std::string_view(_data, length); }

        bool boolean();
        uint8_t octet();
        char character();
        uint16_t ushort();
        uint32_t ulong();
        uint64_t ulonglong();
        // short
        // long
        // longlong
        // float
        // double
        CDRDecoder blob();
        std::string_view string();
        // sequence
        // value
        // object
        // reference

        bool operator==(const CDRDecoder &rhs) const;
        const char *data() const { return _data; }
        void skip(size_t size) { offset += size; }
        void setLittleEndian(bool little) { _endian = little ? std::endian::little : std::endian::big; }
        void setOffset(size_t offset) { this->offset = offset; }
        size_t getOffset() const { return offset; }

    protected:
        void align2() {
            if (offset & 0x01) {
                offset |= 0x01;
                ++offset;
            }
        }
        void align4() {
            if (offset & 0x03) {
                offset |= 0x03;
                ++offset;
            }
        }
        void align8() {
            if (offset & 0x07) {
                offset |= 0x07;
                ++offset;
            }
        }
        const char *ptr2() {
            align2();
            auto ptr = _data + offset;
            offset += 2;
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            return ptr;
        }
        const char *ptr4() {
            align4();
            auto ptr = _data + offset;
            offset += 4;
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            return ptr;
        }
        const char *ptr8() {
            align8();
            auto ptr = _data + offset;
            offset += 8;
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            return ptr;
        }
};

}  // namespace CORBA
