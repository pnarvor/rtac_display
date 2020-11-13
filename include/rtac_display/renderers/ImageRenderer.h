#ifndef _DEF_RTAC_DISPLAY_IMAGE_RENDERER_H_
#define _DEF_RTAC_DISPLAY_IMAGE_RENDERER_H_

#include <rtac_base/types/Handle.h>

#include <rtac_display/renderers/Renderer.h>
#include <rtac_display/views/ImageView.h>

namespace rtac { namespace display {

class ImageRenderer : public Renderer
{
    protected:

    void init_texture();

    public:

    using Ptr      = rtac::types::Handle<ImageRenderer>;
    using ConstPtr = rtac::types::Handle<const ImageRenderer>;

    using Mat4  = ImageView::Mat4;
    using Shape = ImageView::Shape;

    static const std::string vertexShader;
    static const std::string fragmentShader;

    protected:

    GLuint texId_;
    ImageView::Ptr imageView_;

    public:

    static Ptr New();

    ImageRenderer();
    ~ImageRenderer();
    
    void set_image(const Shape& imageSize, GLuint buffer);
    void set_image(const Shape& imageSize, const uint8_t* data);
    template <typename BufferT>
    void set_image(const BufferT& buffer);
    
    virtual void draw();
};

// Implementation
template <typename BufferT>
void ImageRenderer::set_image(const BufferT& buffer)
{
    this->set_image(buffer->shape(), buffer->gl_id());
}

}; //namespace display
}; //namespace rtac

#endif //_DEF_RTAC_DISPLAY_IMAGE_RENDERER_H_
