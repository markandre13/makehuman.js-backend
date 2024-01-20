#pragma once

#include <bit>
#include <cctype>
#include <iostream>
#include <string>

namespace CORBA {

class DataView {
    public:
        const char *_data;
        size_t offset;
        size_t length;
        std::endian _endian;

        DataView(): _data(nullptr), offset(0), length(0) {}
        DataView(const char *data, size_t length, std::endian endian = (std::endian)0) : _data(data), offset(0), length(length), _endian(endian) {}
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
        DataView blob();
        std::string_view string();
        // sequence
        // value
        // object
        // reference

        bool operator==(const DataView &rhs) const;
        const char *data() const { return _data; }
        void skip(size_t size) { offset += size; }
        void setLittleEndian(bool little) { _endian = little ? std::endian::little : std::endian::big; }
        void setOffset(size_t offset) { this->offset = offset; }
        size_t getOffset() const { return offset; }

    protected:
        const char *align2() {
            if (offset & 0x01) {
                offset |= 0x01;
                ++offset;
            }
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            auto ptr = _data + offset;
            offset += 2;
            return ptr;
        }
        const char *align4() {
            if (offset & 0x03) {
                offset |= 0x03;
                ++offset;
            }
            auto ptr = _data + offset;
            offset += 4;
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            return ptr;
        }
        const char *align8() {
            if (offset & 0x07) {
                offset |= 0x07;
                ++offset;
            }
            auto ptr = _data + offset;
            offset += 8;
            if (offset > length) {
                throw std::out_of_range("out of range");
            }
            return ptr;
        }
};

}  // namespace CORBA
