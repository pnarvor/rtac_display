#include <rtac_display/PointCloudRenderer.h>

namespace rtac { namespace display {

const std::string PointCloudRenderer::vertexShader = std::string( R"(
#version 430 core

in vec3 point;

uniform mat4 view;
uniform vec3 color;

out vec3 c;

void main()
{
    gl_Position = view*vec4(point, 1.0f);
    c = color;
}
)");

const std::string PointCloudRenderer::fragmentShader = std::string(R"(
#version 430 core

in vec3 c;
out vec4 outColor;

void main()
{
    outColor = vec4(c, 1.0f);
}
)");

PointCloudRenderer::Ptr PointCloudRenderer::New(const View3D::Ptr& view, const Color& color)
{
    return Ptr(new PointCloudRenderer(view, color));
}

PointCloudRenderer::PointCloudRenderer(const View3D::Ptr& view, const Color& color) :
    Renderer(vertexShader, fragmentShader, view),
    numPoints_(0),
    points_(0),
    pose_(Pose()),
    color_(color),
    normalsRenderer_(NULL)
{
    std::cout << "Request : " << color[0] << ", " << color[1] << ", " << color[2] << std::endl;
    std::cout << "Color : " << color_[0] << ", " << color_[1] << ", " << color_[2] << std::endl;
    this->set_color(color);
    std::cout << "Color : " << color_[0] << ", " << color_[1] << ", " << color_[2] << std::endl;
}

PointCloudRenderer::~PointCloudRenderer()
{
    this->delete_points();
}

void PointCloudRenderer::allocate_points(size_t numPoints)
{
    if(!points_) {
        glGenBuffers(1, &points_);
    }
    if(numPoints_ < numPoints) {
        glBindBuffer(GL_ARRAY_BUFFER, points_);
        glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*numPoints, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void PointCloudRenderer::delete_points()
{
    if(points_ > 0) {
        glDeleteBuffers(1, &points_);
    }
    points_ = 0;
    numPoints_ = 0;
}

void PointCloudRenderer::set_points(size_t numPoints, const float* data)
{
    this->allocate_points(numPoints);
    glBindBuffer(GL_ARRAY_BUFFER, points_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float)*numPoints,
                    static_cast<const void*>(data));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    numPoints_ = numPoints;
}

void PointCloudRenderer::set_points(size_t numPoints, GLuint points)
{
    if(points_ != points) {
        this->delete_points();
        //points_ = buffer->gl_id();
        //numPoints_ = buffer->shape().area();
        points_    = points;
        numPoints_ = numPoints;
    }
}

void PointCloudRenderer::set_normals(size_t numPoints, const float* data,
                                     bool normalizeNormals)
{
    GLuint dnormals;
    glGenBuffers(1, &dnormals);

    glBindBuffer(GL_ARRAY_BUFFER, dnormals);
    glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*numPoints, data,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    this->set_normals(numPoints, dnormals, normalizeNormals);
}

void PointCloudRenderer::set_normals(size_t numPoints, GLuint normals,
                                     bool normalizeNormals)
{
    if(numPoints != numPoints_)
        throw std::runtime_error(
            "PointCloudRendere::set_normals : not as many normals as points.");

    if(!normalsRenderer_) {
        normalsRenderer_ = NormalsRenderer::New(view_, color_);
        normalsRenderer_->set_pose(pose_);
    }
    
    normalsRenderer_->set_normals(numPoints_, points_, normals,
                                  normalizeNormals);
}

void PointCloudRenderer::set_pose(const Pose& pose)
{
    pose_ = pose;
    if(normalsRenderer_)
        normalsRenderer_->set_pose(pose_);
}

void PointCloudRenderer::set_color(const Color& color)
{
    color_[0] = std::max(0.0f, std::min(1.0f, color[0]));
    color_[1] = std::max(0.0f, std::min(1.0f, color[1]));
    color_[2] = std::max(0.0f, std::min(1.0f, color[2]));
}

void PointCloudRenderer::draw()
{
    if(points_ == 0 || numPoints_ == 0)
        return;
    
    glDisable(GL_DEPTH_TEST);
    Mat4 view = view_->view_matrix() * pose_.homogeneous_matrix();

    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(1);

    glUseProgram(renderProgram_);
    
    glBindBuffer(GL_ARRAY_BUFFER, points_);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    //color_[0] = 1.0;
    //color_[1] = 0.0;
    //color_[2] = 0.0;

    glUniformMatrix4fv(glGetUniformLocation(renderProgram_, "view"),
        1, GL_FALSE, view.data());
    glUniform3fv(glGetUniformLocation(renderProgram_, "color"),
        1, color_.data());

    glDrawArrays(GL_POINTS, 0, numPoints_);
    
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);

    glPointSize(pointSize);

    if(normalsRenderer_)
        normalsRenderer_->draw();
}

void PointCloudRenderer::set_view(const View::Ptr& view)
{
    this->Renderer::set_view(view);
    if(normalsRenderer_) {
        normalsRenderer_->set_view(view);
    }
}

}; //namespace display
}; //namespace rtac
