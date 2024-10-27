#include "../src/freemocap/freemocap.hh"

#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;
using namespace std;

void handle_error(const char *txt) {}

kaffeeklatsch_spec([] {
    describe("class FreeMoCap", [] {
        it("parse CSV file", [] {
            BlazePose pose;
            FreeMoCap capture("freemocap/mediapipe_body_3d_xyz.csv");

            expect(capture.isEof()).to.be.beFalse();

            capture.getPose(&pose);

            expect(capture.isEof()).to.be.beFalse();
            expect(pose.landmarks[0]).to.equal(1432.7336141407498);
            expect(pose.landmarks[98]).to.equal(2046.9937332708537);

            capture.getPose(&pose);

            expect(capture.isEof()).to.be.beFalse();
            expect(pose.landmarks[0]).to.equal(3432.7336183839243);
            expect(pose.landmarks[98]).to.equal(4046.9937372298077);

            capture.getPose(&pose);

            expect(capture.isEof()).to.be.beTrue();
            expect(pose.landmarks[0]).to.equal(1432.7336141407498);
            expect(pose.landmarks[98]).to.equal(2046.9937332708537);
        });
    });
    describe("class MoCap", [] {
        it("cache motion capture", [] {
            // FreeMoCap capture("freemocap/mediapipe_body_3d_xyz.csv");
            // MoCap mocap(capture);
            MoCap mocap(FreeMoCap("freemocap/mediapipe_body_3d_xyz.csv"));
            expect(mocap.size()).to.equal(2);

            expect(mocap[0].landmarks[0]).to.equal(1432.7336141407498);
            expect(mocap[0].landmarks[98]).to.equal(2046.9937332708537);

            expect(mocap[1].landmarks[0]).to.equal(3432.7336183839243);
            expect(mocap[1].landmarks[98]).to.equal(4046.9937372298077);
        });
    });
});
