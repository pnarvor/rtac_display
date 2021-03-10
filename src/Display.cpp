#include <rtac_display/Display.h>

namespace rtac { namespace display {

Display::Display(int width, int height, const std::string& title) :
    window_(NULL),
    frameCounterEnabled_(true)
{
    if(!glfwInit()) {
        throw std::runtime_error("GLFW initialization failure.");
    }
    window_ = Window(glfwCreateWindow(width, height, title.c_str(), NULL, NULL),
                     glfwDestroyWindow); //custom deleter
    if(!window_) {
        throw std::runtime_error("GLFW window creation failure.");
    }
    glfwMakeContextCurrent(window_.get());
    
    // init glew (no gl function availabel if not done)
    GLenum initGlewStatus(glewInit());
    if(initGlewStatus != GLEW_OK)
        std::cout << "Failed to initialize glew" << std::endl;
    //std::cout << glGetString(GL_VERSION) << std::endl;
    //if(GLEW_ARB_compute_shader)
    //    std::cout << "Compute shader ok !" << std::endl;

    glClearColor(0.0,0.0,0.0,1.0);
    //glClearColor(0.7,0.7,0.7,1.0);

    // to measure fps
    glfwSwapInterval(0);

    glViewport(0.0,0.0,width,height);
}

void Display::terminate()
{
    glfwTerminate();
}

Display::Shape Display::window_shape() const
{
    Shape wSize;
    int width, height;
    glfwGetWindowSize(window_.get(), &width, &height);
    wSize.width  = width;
    wSize.height = height;
    return wSize;
}

int Display::should_close() const
{
    glfwPollEvents();
    return glfwWindowShouldClose(window_.get()) > 0;
}

int Display::is_drawing()
{
    this->draw();
    glfwPollEvents();
    return glfwWindowShouldClose(window_.get()) == 0;
}

void Display::wait_for_close() const
{
    using namespace std;
    while(!this->should_close()) {
        std::this_thread::sleep_for(100ms);
    }
}

void Display::add_view(const View::Ptr& view)
{
    for(auto v : views_) {
        if(view.get() == view.get()) {
            return;
        }
    }
    views_.push_back(view);
}

void Display::add_renderer(const Renderer::Ptr& renderer)
{
    renderers_.push_back(renderer);
    views_.push_back(renderer->view());
}

void Display::draw()
{
    glfwMakeContextCurrent(window_.get());
    Shape wSize = this->window_shape();

    for(auto view : views_) {
        view->set_screen_size(wSize);
    }

    glViewport(0,0,wSize.width,wSize.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(auto renderer : renderers_) {
        if(renderer) {
            renderer->draw();
        }
    }

    glfwSwapBuffers(window_.get());

    if(frameCounterEnabled_) {
        std::cout << frameCounter_;
    }
}

void Display::enable_frame_counter()
{
    frameCounterEnabled_ = true;
}

void Display::disable_frame_counter()
{
    frameCounterEnabled_ = false;
}

}; //namespace display
}; //namespace rtac


