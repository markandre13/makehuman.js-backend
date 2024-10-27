#include "freemocap.hh"
#include "../util.hh"

using namespace std;

void FreeMoCap::getPose(BlazePose *blazepose) {
    std::string line;
    eof = false;
    if (!getline(in, line) || trim(line).size() == 0) {
        eof = true;
        in.clear();
        in.seekg(0);
        getline(in, line);
        getline(in, line);
    }

    size_t bow = 0, i = 0;
    while (true) {
        size_t eow = line.find(',', bow);

        auto column = line.substr(bow, eow - bow);
        auto f = stof(column.substr(0, column.size() - 2));
        blazepose->landmarks[i] = f;
        if (eow == string::npos) {
            break;
        }
        ++i;
        bow = eow + 1;
    }
}

MoCap::MoCap(FreeMoCap &&mocap) {
    while(!mocap.isEof()) {
        store.resize(store.size()+1);
        mocap.getPose(&store.back());
    }
    store.resize(store.size()-1);
}
