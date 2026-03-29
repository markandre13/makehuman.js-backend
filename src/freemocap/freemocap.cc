#include "freemocap.hh"

#include "../util.hh"

using namespace std;

FreeMoCap::FreeMoCap(const std::string& filename) : in(filename), eof(false) {
    if (!in) {
        throw std::runtime_error(format("failed to open file '{}'", filename));
    }
    // skip csv header
    std::string line;
    getline(in, line);
    if (!line.starts_with("body_nose_x,body_nose_y,body_nose_z,body_left_eye_inner_x,body_left_eye_inner_y,body_left_eye_inner_z,body_left_eye_x,body_left_eye_y,body_left_eye_z,body_left_eye_outer_x,body_left_eye_outer_y,body_left_eye_outer_z,body_right_eye_inner_x,body_right_eye_inner_y,body_right_eye_inner_z,body_right_eye_x,body_right_eye_y,body_right_eye_z,body_right_eye_outer")) {
        throw runtime_error(format("unexpected header in file mediapipe_body_3d_xyz.csv file ({})", filename));
    }
    // println("got line '{}'", line);
}

void FreeMoCap::getPose(BlazePose* blazepose) {
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

MoCap::MoCap(FreeMoCap&& mocap) {
    while (!mocap.isEof()) {
        store.resize(store.size() + 1);
        mocap.getPose(&store.back());
    }
    store.resize(store.size() - 1);
}
