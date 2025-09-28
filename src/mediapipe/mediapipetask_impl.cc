#include "mediapipetask_impl.hh"
#if HAVE_MEDIAPIPE
#include "pose.hh"
#include "face.hh"
#endif

using namespace std;

std::vector<std::shared_ptr<MediaPipeTask>> getMediaPipeTasks(std::shared_ptr<CORBA::ORB> orb, Backend_impl *backend) {
    std::vector<std::shared_ptr<MediaPipeTask>> tasks;
    
#if HAVE_MEDIAPIPE
    auto pose = make_shared<MediapipePose>(backend);
    orb->activate_object(pose);
    tasks.push_back(pose);

    auto face = make_shared<MediapipeFace>(backend);
    orb->activate_object(face);
    tasks.push_back(face);
#endif

    return tasks;
}
