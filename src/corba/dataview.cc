#include "dataview.hh"

void hexdump(unsigned char *buffer, int received);

namespace CORBA {

bool DataView::operator==(const DataView &rhs) const {
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

bool DataView::boolean() {
    auto value = _data[offset] != 0;
    offset += 1;
    return value;
}

uint8_t DataView::octet() {
    auto value = _data[offset];
    offset += 1;
    return value;
}

char DataView::character() {
    auto value = _data[offset];
    offset += 1;
    return value;
}

uint16_t DataView::ushort() {
    auto value = *reinterpret_cast<const uint16_t *>(align2());
    if (std::endian::native != _endian) {
        value = __builtin_bswap16(value);
    }
    return value;
}

uint32_t DataView::ulong() {
    auto ptr = reinterpret_cast<const uint32_t *>(align4());
    uint32_t value = *ptr;
    if (std::endian::native != _endian) {
        value = __builtin_bswap32(value);
    }
    return value;
}

uint64_t DataView::ulonglong() {
    auto value = *reinterpret_cast<const uint64_t *>(align8());
    if (std::endian::native != _endian) {
        value = __builtin_bswap64(value);
    }
    return value;
}

DataView DataView::blob() {
    size_t len = ulong();
    DataView result(_data + offset, len);
    offset += len;
    if (offset > length) {
        throw std::out_of_range("out of range");
    }
    return result;
}

std::string_view DataView::string() {
    size_t len = ulong();
    std::string_view result(_data + offset, len-1);
    offset += len;
    if (offset > length) {
        throw std::out_of_range("out of range");
    }
    return result;
}

}  // namespace CORBA
