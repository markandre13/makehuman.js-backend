#include "mediapipetask_impl.hh"
#include "pose.hh"
#include "face.hh"

using namespace std;

std::vector<std::shared_ptr<MediaPipeTask>> getMediaPipeTasks(std::shared_ptr<CORBA::ORB> orb, Backend_impl *backend) {
    std::vector<std::shared_ptr<MediaPipeTask>> tasks;
    
    auto pose = make_shared<MediapipePose>(backend);
    orb->activate_object(pose);
    tasks.push_back(pose);

    auto face = make_shared<MediapipeFace>(backend);
    orb->activate_object(face);
    tasks.push_back(face);

    return tasks;
}
