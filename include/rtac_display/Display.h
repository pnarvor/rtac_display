#ifndef _DEF_RTAC_DISPLAY_H_
#define _DEF_RTAC_DISPLAY_H_

#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>

#include <GL/glew.h>
//#define GL3_PROTOTYPES 1
#include <GL/gl.h>

//#include <glm/glm.hpp>
//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include <rtac_base/time.h>
#include <rtac_base/types/CallbackQueue.h>

#include <rtac_display/utils.h>
#include <rtac_display/views/View.h>
#include <rtac_display/renderers/Renderer.h>
#include <rtac_display/EventHandler.h>

namespace rtac { namespace display {

class Display
{
    public:

    using Window    = std::shared_ptr<GLFWwindow>;
    using Shape     = View::Shape;
    using Views     = std::vector<View::Ptr>;
    using Renderers = std::vector<Renderer::Ptr>;

    using KeyCallbacks           = rtac::types::CallbackQueue<int,int,int,int>;
    using MousePositionCallbacks = rtac::types::CallbackQueue<double,double>;
    using MouseButtonCallbacks   = rtac::types::CallbackQueue<int,int,int>;
    using ScrollCallbacks        = rtac::types::CallbackQueue<double,double>;

    using KeyCallbackT           = KeyCallbacks::CallbackT;
    using MousePositionCallbackT = MousePositionCallbacks::CallbackT;
    using MouseButtonCallbackT   = MouseButtonCallbacks::CallbackT;
    using ScrollCallbackT        = ScrollCallbacks::CallbackT;

    protected:
    
    Window       window_;
    Views        views_;
    Renderers    renderers_;
    EventHandler eventHandler_;
    
    rtac::time::FrameCounter frameCounter_;
    bool frameCounterEnabled_;

    // Event callback queues
    KeyCallbacks           keyCallbacks_;
    MousePositionCallbacks mousePositionCallbacks_;
    MouseButtonCallbacks   mouseButtonCallbacks_;
    ScrollCallbacks        scrollCallbacks_;

    public:

    Display(int width = 800, int height = 600,
            const std::string& title = "optix render");
    void terminate();

    Shape window_shape() const;
    int should_close() const;
    int is_drawing();
    void wait_for_close() const;
    
    void add_view(const View::Ptr& view);
    void add_renderer(const Renderer::Ptr& renderer);
    virtual void draw();

    void enable_frame_counter();
    void disable_frame_counter();
    void limit_frame_rate(double fps);
    void free_frame_rate();

    // event related methods
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int modes);
    static void mouse_position_callback(GLFWwindow* window, double x, double y);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double x, double y);

    unsigned int add_key_callback(const KeyCallbackT& callback);
    unsigned int add_mouse_position_callback(const MousePositionCallbackT& callback);
    unsigned int add_mouse_button_callback(const MouseButtonCallbackT& callback);
    unsigned int add_scroll_callback(const ScrollCallbackT& callback);
    
    void add_event_handler(const EventHandler::Ptr& eventHandler);
};

}; //namespace display
}; //namespace rtac


#endif //_DEF_OPTIX_HELPERS_DISPLAY_H_
