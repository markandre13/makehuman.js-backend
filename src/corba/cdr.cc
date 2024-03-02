#include "cdr.hh"

#include <iostream>
#include <format>

using namespace std;

namespace CORBA {

void CDREncoder::boolean(bool value) {
    reserve(offset + 1);
    auto ptr = reinterpret_cast<uint8_t *>(_data.data() + offset);
    offset += 1;
    *ptr = value ? 1 : 0;
}

void CDREncoder::octet(uint8_t value) {
    reserve(offset + 1);
    auto ptr = reinterpret_cast<uint8_t *>(_data.data() + offset);
    offset += 1;
    *ptr = value;
}

void CDREncoder::ushort(uint16_t value) {
    align2();
    reserve(offset + 2);
    auto ptr = reinterpret_cast<uint16_t *>(_data.data() + offset);
    offset += 2;
    *ptr = value;
}

void CDREncoder::ulong(uint32_t value) {
    align4();
    reserve(offset + 4);
    auto ptr = reinterpret_cast<uint32_t *>(_data.data() + offset);
    offset += 4;
    *ptr = value;
}

void CDREncoder::ulonglong(uint64_t value) {
    align8();
    reserve(offset + 8);
    auto ptr = reinterpret_cast<uint64_t *>(_data.data() + offset);
    offset += 8;
    *ptr = value;
}

void CDREncoder::blob(const char *value, size_t nbytes) {
    ulong(nbytes);
    reserve(offset + nbytes);
    memcpy(_data.data() + offset, value, nbytes);
    offset += nbytes;
}
void CDREncoder::string(const char *value) {
    string(value, strlen(value));
}
void CDREncoder::string(const char *value, size_t nbytes) {
    ulong(nbytes + 1);
    reserve(offset + nbytes + 1);
    memcpy(_data.data() + offset, value, nbytes);
    offset += nbytes;
    _data[offset] = 0;
    ++offset;
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
    auto value = _data[m_offset] != 0;
    m_offset += 1;
    return value;
}

uint8_t CDRDecoder::octet() {
    auto value = _data[m_offset];
    m_offset += 1;
    return value;
}

char8_t CDRDecoder::character() {
    auto value = (char8_t)_data[m_offset];
    m_offset += 1;
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

CORBA::blob CDRDecoder::blob() {
    size_t len = ulong();
    auto buffer = _data + m_offset;
    m_offset += len;
    if (m_offset > length) {
        throw std::out_of_range("out of range");
    }
    return CORBA::blob(buffer, len);
}

std::string CDRDecoder::string() {
    return string(ulong());
}

std::string CDRDecoder::string(size_t len) {
    auto buffer = _data + m_offset;
    m_offset += len;
    if (m_offset > length) {
        throw std::out_of_range("out of range");
    }
    return std::string(buffer, len-1);
}

CORBA::blob_view CDRDecoder::blob_view() {
    size_t len = ulong();
    auto buffer = _data + m_offset;
    m_offset += len;
    if (m_offset > length) {
        throw std::out_of_range("out of range");
    }
    return CORBA::blob_view(buffer, len);
}

std::string_view CDRDecoder::string_view() {
    return string_view(ulong());
}

std::string_view CDRDecoder::string_view(size_t len) {
    auto buffer = _data + m_offset;
    m_offset += len;
    if (m_offset > length) {
        throw std::out_of_range("out of range");
    }
    return std::string_view(buffer, len-1);
}

}  // namespace CORBA
