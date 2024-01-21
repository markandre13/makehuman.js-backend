#pragma once

#include <bit>
#include <cctype>
#include <string>
#include <vector>

namespace CORBA {

// Common Data Representation

class CDREncoder {
    public:
        std::vector<char> _data;
        size_t offset = 0;
    protected: 
        std::vector<size_t> sizeStack;
    public:
        void octet(u_int8_t);
        void ushort(uint16_t);
        void ulong(uint32_t);
        void ulonglong(uint64_t);

        void string(const char *string);
        void string(const char *buffer, size_t size);
        void string(std::string_view &string);
        void string(std::string &string);
        void blob(const char *buffer, size_t size) { string(buffer, size); }
        void endian();

        void reserveSize();
        void fillInSize();

        const char *data() { return _data.data(); }
        size_t length() { return _data.size(); }

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
        char8_t character();
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
