#include "cdr.hh"

#include <iostream>
#include <format>

using namespace std;

namespace CORBA {

void CDREncoder::boolean(bool value) {
    _data.resize(offset + 1);
    auto ptr = reinterpret_cast<uint8_t *>(_data.data() + offset);
    offset += 1;
    *ptr = value ? 1 : 0;
}

void CDREncoder::octet(uint8_t value) {
    _data.resize(offset + 1);
    auto ptr = reinterpret_cast<uint8_t *>(_data.data() + offset);
    offset += 1;
    *ptr = value;
}

void CDREncoder::ushort(uint16_t value) {
    align2();
    _data.resize(offset + 2);
    auto ptr = reinterpret_cast<uint16_t *>(_data.data() + offset);
    offset += 2;
    *ptr = value;
}

void CDREncoder::ulong(uint32_t value) {
    align4();
    _data.resize(offset + 4);
    auto ptr = reinterpret_cast<uint32_t *>(_data.data() + offset);
    offset += 4;
    *ptr = value;
}

void CDREncoder::ulonglong(uint64_t value) {
    align8();
    _data.resize(offset + 8);
    auto ptr = reinterpret_cast<uint64_t *>(_data.data() + offset);
    offset += 8;
    *ptr = value;
}
void CDREncoder::string(const char *buffer) {
    string(buffer, strlen(buffer) +1 );
}
void CDREncoder::string(const char *string, size_t size) {
    ulong(size);
    _data.resize(offset + size);
    memcpy(_data.data() + offset, string, size);
    offset += size;
}
void CDREncoder::string(const std::string_view &value) {
    ulong(value.size() + 1);
    _data.resize(offset + value.size() + 1);
    memcpy(_data.data() + offset, value.data(), value.size());
    _data[_data.size() - 1] = 0;
    offset += value.size() + 1;
}
void CDREncoder::string(const std::string &value) {
    ulong(value.size() + 1);
    _data.resize(offset + value.size() + 1);
    memcpy(_data.data() + offset, value.data(), value.size());
    _data[_data.size() - 1] = 0;
    offset += value.size() + 1;
}
void CDREncoder::endian() { octet(endian::native == endian::big ? 0 : 1); }

void CDREncoder::reserveSize() {
    align4();
    offset += 4;
    sizeStack.push_back(offset);
}
void CDREncoder::fillInSize() {
    if (sizeStack.empty()) {
        throw runtime_error("internal error: fillinSize() misses reserveSize()");
    }
    auto currrentOffset = offset;
    auto savedOffset = sizeStack.back();
    sizeStack.pop_back();
    offset = savedOffset - 4;
    auto size = currrentOffset - savedOffset;
    ulong(size);
    offset = currrentOffset;
}

bool CDRDecoder::operator==(const CDRDecoder &rhs) const {
    if (this == &rhs) {
        return true;
    }
    if (length != rhs.length) {
        return false;
    }
    if (_data == rhs._data) {
        return true;
    }
    return memcmp(_data, rhs._data, length) == 0;
}

bool CDRDecoder::boolean() {
    auto value = _data[offset] != 0;
    offset += 1;
    return value;
}

uint8_t CDRDecoder::octet() {
    auto value = _data[offset];
    offset += 1;
    return value;
}

char8_t CDRDecoder::character() {
    auto value = (char8_t)_data[offset];
    offset += 1;
    return value;
}

uint16_t CDRDecoder::ushort() {
    auto value = *reinterpret_cast<const uint16_t *>(ptr2());
    if (std::endian::native != _endian) {
        value = __builtin_bswap16(value);
    }
    return value;
}

uint32_t CDRDecoder::ulong() {
    auto ptr = reinterpret_cast<const uint32_t *>(ptr4());
    uint32_t value = *ptr;
    if (std::endian::native != _endian) {
        value = __builtin_bswap32(value);
    }
    return value;
}

uint64_t CDRDecoder::ulonglong() {
    auto value = *reinterpret_cast<const uint64_t *>(ptr8());
    if (std::endian::native != _endian) {
        value = __builtin_bswap64(value);
    }
    return value;
}

string CDRDecoder::blob() {
    size_t len = ulong();
    std::string result(_data + offset, len);
    offset += len;
    if (offset > length) {
        throw std::out_of_range("out of range");
    }
    return result;
}

std::string CDRDecoder::string() {
    return string(ulong());
}

std::string CDRDecoder::string(size_t len) {
    std::string result(_data + offset, len - 1);
    offset += len;
    if (offset > length) {
        throw std::out_of_range("out of range");
    }
    return result;
}

}  // namespace CORBA
