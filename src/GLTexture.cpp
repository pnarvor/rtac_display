#include <rtac_display/GLTexture.h>

namespace rtac { namespace display {

/**
 * @return a shared pointer to a newly created GLTexture.
 */
GLTexture::Ptr GLTexture::New()
{
    return Ptr(new GLTexture());
}

/**
 * Contructor of GLTexture
 *
 * An OpenGL context must have been created beforehand.
 */
GLTexture::GLTexture() :
    shape_({0,0}),
    texId_(0),
    format_(GL_RGBA)
{
    this->init_texture();
    this->GLTexture::configure_texture();
}

GLTexture::~GLTexture()
{
    this->delete_texture();
}

GLTexture::GLTexture(GLTexture&& other) :
    shape_ (std::move(other.shape_)),
    texId_ (std::exchange(other.texId_, 0)),
    format_(other.format_)
{}

GLTexture& GLTexture::operator=(GLTexture&& other)
{
    shape_  = std::move(other.shape_);
    texId_  = std::exchange(other.texId_, 0);
    format_ = other.format_;

    return *this;
}

/**
 * Creates a new OpenGL Texture Object.
 *
 * Must not be called by the user.
 */
void GLTexture::init_texture()
{
    if(!texId_)
        glGenTextures(1, &texId_);
}

/**
 * Free the OpenGL Texture Object.
 *
 * It is safe to call this function several times in a row.
 */
void GLTexture::delete_texture()
{
    if(texId_)
        glDeleteTextures(1, &texId_);
    texId_ = 0;
    shape_ = Shape({0,0});
}

/**
 * Default OpenGL Texture configuration (configure min-mag filters).
 */
void GLTexture::configure_texture()
{
    glBindTexture(GL_TEXTURE_2D, texId_);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @return the dimension of the texture.
 */
Shape GLTexture::shape() const
{
    return shape_;
}

size_t GLTexture::width() const
{
    return shape_.width;
}

size_t GLTexture::height() const
{
    return shape_.height;
}

/**
 * @return the id of the OpenGL Texture Object.
 */
GLuint GLTexture::gl_id() const
{
    return texId_;
}

/**
 * Returns the pixel format of the texture.
 *
 * The pixel format is infered at compile time through the use of the GLFormat
 * structures. It can be any value amongs GL_RED, GL_RG, GL_RGB, GL_RGBA. See
 * more information
 * [here](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml).
 *
 * @return the pixel format of the texture.
 */
GLint GLTexture::format() const
{
    return format_;
}

void GLTexture::bind(GLenum target)
{
    glBindTexture(target, this->gl_id());
}

void GLTexture::unbind(GLenum target)
{
    glBindTexture(target, 0);
}

/**
 * Creates a new texture from a PPM image file.
 *
 * An OpenGL context must have been created beforehand.
 *
 * @param path Path to .ppm image file.
 *
 * @return a shared pointer to a newly created texture.
 */
GLTexture::Ptr GLTexture::from_ppm(const std::string& path)
{
    auto texture = GLTexture::New();

    size_t Win, Hin;
    std::vector<uint8_t> data;

    files::read_ppm(path, Win, Hin, data); 

    texture->set_image({Win, Hin}, (const rtac::types::Point3<unsigned char>*)data.data());
    return texture;
}

#ifdef RTAC_PNG
/**
 * Creates a new texture from a PNG image file.
 *
 * An OpenGL context must have been created beforehand.
 *
 * @param path Path to .png image file.
 *
 * @return a shared pointer to a newly created texture.
 */
GLTexture::Ptr GLTexture::from_png(const std::string& path)
{
    auto texture = GLTexture::New();

    rtac::external::PNGCodec codec;
    codec.read_image(path, true);

    GLint  internalFormat;
    GLenum pixelFormat;
    switch(codec.channels()) {
        default:
            std::runtime_error("GLTexture::from_png : unhandled channel count.");
            break;
        case 1: internalFormat = GL_RED;  pixelFormat = GL_RED;  break;
        case 2: internalFormat = GL_RG;   pixelFormat = GL_RG;   break;
        case 3: internalFormat = GL_RGB;  pixelFormat = GL_RGB;  break;
        case 4: internalFormat = GL_RGBA; pixelFormat = GL_RGBA; break;
    }
    GLenum scalarType;
    switch(codec.bitdepth()) {
        default:
            std::runtime_error("GLTexture::from_png : unhandled bit depth.");
            break;
        case  8:  scalarType = GL_UNSIGNED_BYTE;  break;
        case 16:  scalarType = GL_UNSIGNED_SHORT; break;
    }


    texture->set_image({codec.width(), codec.height()},
                       internalFormat, pixelFormat, scalarType, 
                       codec.data().data());
    return texture;
}
#endif // RTAC_PNG
GLTexture::Ptr GLTexture::from_file(const std::string& path)
{
    auto texture = GLTexture::New();

    rtac::external::ImageCodec codec;
    auto img = codec.read_image(path, true);

    GLint  internalFormat;
    GLenum pixelFormat;
    switch(img->channels()) {
        default:
            std::runtime_error("GLTexture::from_png : unhandled channel count.");
            break;
        case 1: internalFormat = GL_RED;  pixelFormat = GL_RED;  break;
        case 2: internalFormat = GL_RG;   pixelFormat = GL_RG;   break;
        case 3: internalFormat = GL_RGB;  pixelFormat = GL_RGB;  break;
        case 4: internalFormat = GL_RGBA; pixelFormat = GL_RGBA; break;
    }
    GLenum scalarType;
    switch(img->bitdepth()) {
        default:
            std::runtime_error("GLTexture::from_png : unhandled bit depth.");
            break;
        case  8:  scalarType = GL_UNSIGNED_BYTE;  break;
        case 16:  scalarType = GL_UNSIGNED_SHORT; break;
    }


    texture->set_image({img->width(), img->height()},
                       internalFormat, pixelFormat, scalarType, 
                       img->data().data());
    return texture;
}


}; //namespace display
}; //namespace rtac


