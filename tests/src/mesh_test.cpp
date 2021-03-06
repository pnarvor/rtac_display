#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
using namespace std;

#include <rtac_base/time.h>
using FrameCounter = rtac::time::FrameCounter;

#include <rtac_base/types/Pose.h>
#include <rtac_base/types/Mesh.h>
using Pose = rtac::types::Pose<float>;
using Quaternion = rtac::types::Quaternion<float>;
using Mesh = rtac::types::Mesh<>;

#include <rtac_display/Display.h>
#include <rtac_display/views/PinholeView.h>
#include <rtac_display/renderers/MeshRenderer.h>
using namespace rtac::display;

int main()
{
    int W = 1920, H = 1080;

    Display display;
    
    auto view3d = PinholeView::New();
    view3d->look_at({0,0,0}, {5,4,3});
    auto renderer = display.create_renderer<Renderer>(view3d);

    auto meshRenderer = display.create_renderer<MeshRenderer>(view3d);
    meshRenderer->mesh() = GLMesh::cube();
    meshRenderer->set_pose(Pose({0,0,3}));
    meshRenderer->set_color({1,1,0,1});

    float dangle = 0.001;
    Pose R({0.0,0.0,0.0}, Quaternion({cos(dangle/2), 0.0, 0.0, sin(dangle/2)}));
    
    FrameCounter counter;
    while(!display.should_close()) {
        view3d->set_pose(R * view3d->pose());
        
        display.draw();
        
        //cout << counter;
        this_thread::sleep_for(10ms);
    }
    cout << endl;

    return 0;
}


