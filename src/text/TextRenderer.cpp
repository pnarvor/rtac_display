#include <rtac_display/text/TextRenderer.h>

namespace rtac { namespace display { namespace text {

/**
 * Point position computed on CPU side. 
 */
const std::string TextRenderer::vertexShader = std::string( R"(
#version 430 core

in vec4 point;
in vec2 uvIn;
out vec2 uv;

void main()
{
    gl_Position = point;
    uv = uvIn;
}
)");

/**
 * Simply outputs the texture value at given texture coordinates.
 */
const std::string TextRenderer::fragmentShader = std::string(R"(
#version 430 core

in vec2 uv;
uniform sampler2D tex;

out vec4 outColor;

void main()
{
    outColor = texture(tex, uv);
}
)");
TextRenderer::TextRenderer(const FontFace::ConstPtr& font) :
    Renderer(vertexShader, fragmentShader),
    font_(font),
    origin_({0,0,0,1})
{
    if(!font_) {
        std::ostringstream oss;
        oss << "Error rtac_display::text::TextRenderer : "
            << "Invalid font face pointer.";
        throw std::runtime_error(oss.str());
    }
}

TextRenderer::Ptr TextRenderer::Create(const FontFace::ConstPtr& font,
                                       const std::string& text)
{
    auto renderer = Ptr(new TextRenderer(font));
    renderer->set_text(text);
    return renderer;
}

void TextRenderer::set_text(const std::string& text)
{
    text_ = text;
    this->update_texture();
}

Shape TextRenderer::compute_text_area(const std::string& text)
{
    if(!font_ || text.size() == 0)
        return Shape({0,0});

    int lineCount = 1;
    float maxWidth = 0.0f, currentWidth = 0.0f;
    for(auto c : text) {
        if(c == '\n') {
            lineCount++;
            maxWidth = std::max(maxWidth, currentWidth);
            currentWidth = 0.0f;
            continue;
        }
        if(c < 32 || c == 127) {
            // non-printable characters
            continue;
        }
        try {
            currentWidth += font_->glyph(c).advance().x;
        }
        catch(const std::out_of_range&) {
            currentWidth += font_->glyph('\n').advance().x;
        }
    }
    maxWidth = std::max(maxWidth, currentWidth);
    return Shape({(size_t)(4*(((int)maxWidth + 3) / 4)),
                  (size_t)(lineCount * font_->baselineskip() + 1)});
}

void TextRenderer::update_texture()
{
    Shape textArea = this->compute_text_area(text_);
    std::cout << "Text area size : " << textArea << std::endl;
    texture_.set_size<types::Point4<float>>(textArea);

    // Preparing a framebuffer for off-screen rendering
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_.gl_id(), 0);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream oss;
        oss << "TextRenderer error : something went wrong when creating a framebuffer "
            << "(GL error : 0x" << std::hex << glGetError() << ")";
        throw std::runtime_error(oss.str());
    }
    texture_.unbind(GL_TEXTURE_2D);

    glClearColor(0.0,0.0,0.0,0.0);
    //glClearColor(0.0,1.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, texture_.shape().width, texture_.shape().height);
    //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    Mat4 origin  = this->view_matrix();
    Mat4 current = Mat4::Identity();
    
    const Glyph* glyph = nullptr;
    for(auto c : text_) {
        if(c == '\n') {
            current(1,3) -= font_->baselineskip();
            current(0,3)  = 0.0f;
            continue;
        }
        if(c < 32 || c == 127) {
            continue;
        }
        try {
            glyph = &font_->glyph(c);
        }
        catch(const std::out_of_range&) {
            // fallback to another glyph if not available
            glyph = &font_->glyph('\n');
        }
        
        std::cout << "character : " << c
                  << ", shape : " << glyph->shape()
                  << ", texture shape : " << glyph->texture().shape() << std::endl;
        glyph->draw(origin * current, {1,1,1});
        current(0,3) += glyph->advance().x;
    }

    // unbinding the frame buffer for re-enabling on-screen rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GL_CHECK_LAST();
}

const std::string& TextRenderer::text() const
{
    return text_;
}

FontFace::ConstPtr TextRenderer::font() const
{
    return font_;
}

const GLTexture& TextRenderer::texture() const
{
    return texture_;
}

/**
 * Return a view matrix transforming from 2D pixel space of the text areato
 * OpenGL clip space [-1,1] with origin at the left end of the first baseline.
 */ 
TextRenderer::Mat4 TextRenderer::view_matrix() const
{
    Mat4 view = Mat4::Identity();
    view(0,0) = 2.0f / texture_.width();
    view(1,1) = -2.0f / texture_.height();

    view(0,3) = -1.0f;
    view(1,3) = -(texture_.height() - 2.0f*font_->ascender()) / texture_.height();

    return view;
}

TextRenderer::Vec4& TextRenderer::origin()
{
    return origin_;
}

const TextRenderer::Vec4& TextRenderer::origin() const
{
    return origin_;
}

void TextRenderer::draw()
{
    // OpenGL clip space origin.
    Vec4 clipOrigin = view_->view_matrix() * origin_;
    float clipWidth  = (2.0f*texture_.width() ) / view_->screen_size().width;
    float clipHeight = (2.0f*texture_.height()) / view_->screen_size().height;

    std::vector<Vec4> points_({
        Vec4(clipOrigin + Vec4({0,0,0,0})),
        Vec4(clipOrigin + Vec4({clipWidth,0,0,0})),
        Vec4(clipOrigin + Vec4({clipWidth,clipHeight,0,0})),
        Vec4(clipOrigin + Vec4({0,clipHeight,0,0}))});
    static const float uv[] = {0,1,
                               1,1,
                               1,0,
                               0,0};
    static const unsigned int indexes[] = {0, 1, 2,
                                           0, 2, 3};

    glUseProgram(renderProgram_);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, points_.data());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(1);

    glUniform1i(glGetUniformLocation(renderProgram_, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_.gl_id());
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indexes);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glUseProgram(0);

    GL_CHECK_LAST();
}

}; //namespace text
}; //namespace display
}; //namespace rtac


