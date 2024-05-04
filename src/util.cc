#include "util.hh"
#include <print>

using namespace std;

vector<string_view> split(const string_view &data, char delimiter) {
    vector<string_view> result;
    size_t bol = 0;
    while (true) {
        size_t eol = data.find(delimiter, bol);
        if (eol == string::npos) {
            result.push_back(data.substr(bol));
            break;
        }
        if (bol != eol) {
            result.push_back(data.substr(bol, eol - bol));
        }
        bol = eol + 1;
    }
    return result;
}

string_view trim(const string_view &data) {
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
    return data.substr(bol, eol+1);
}
