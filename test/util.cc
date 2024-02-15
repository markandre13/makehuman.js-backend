#include "util.hh"

using namespace std;

void sendChordata(void*, unsigned long) {}

vector<uint8_t> parseOmniDump(const string_view &data) {
    vector<uint8_t> result;
    auto lines = split(trim(data));
    string hex("00");
    for (auto row : lines) {
        row = trim(row);
        for (auto i = 0; i < 8; ++i) {
            unsigned idx = i * 5;
            hex[0] = row[idx];
            hex[1] = row[idx + 1];
            if (isspace(hex[0])) {
                break;
            }
            result.push_back(stoi(hex, nullptr, 16));
            hex[0] = row[idx + 2];
            hex[1] = row[idx + 3];
            result.push_back(stoi(hex, nullptr, 16));
        }
    }
    return result;
}

vector<string_view> split(string_view data) {
    vector<string_view> result;
    size_t bol = 0;
    while (true) {
        size_t eol = data.find('\n', bol);
        if (eol == string::npos) {
            result.push_back(data.substr(bol));
            break;
        }
        result.push_back(data.substr(bol, eol - bol));
        bol = eol + 1;
    }
    return result;
}

string_view trim(string_view data) {
    size_t bol = 0;
    while (bol < data.length()) {
        if (!isspace(data[bol])) {
            break;
        }
        ++bol;
    }
    size_t eol = data.length();
    if (bol == eol) {
        return string_view();
    }
    while (eol > 0) {
        --eol;
        if (!isspace(data[eol])) {
            break;
        }
    }
    return data.substr(bol, eol);
}
