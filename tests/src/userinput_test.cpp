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
    display.disable_frame_counter();
    
    auto renderer = Renderer::New();
    auto view3d = PinholeView::New();
    view3d->look_at({0,0,0}, {5,4,3});
    renderer->set_view(view3d);
    display.add_renderer(renderer);

    auto meshRenderer = MeshRenderer::New(view3d);
    meshRenderer->set_mesh(Mesh::cube(0.5));
    meshRenderer->set_pose(Pose({0,0,3}));
    display.add_renderer(meshRenderer);

    float dangle = 0.001;
    Pose R({0.0,0.0,0.0}, Quaternion({cos(dangle/2), 0.0, 0.0, sin(dangle/2)}));
    
    while(!display.should_close()) {
        view3d->set_pose(R * view3d->pose());
        
        display.draw();
        
        this_thread::sleep_for(10ms);
    }

    return 0;
}

