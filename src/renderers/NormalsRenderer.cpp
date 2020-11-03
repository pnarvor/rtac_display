#include <rtac_display/NormalsRenderer.h>

namespace rtac { namespace display {

const std::string NormalsRenderer::vertexShader = std::string(R"(
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

const std::string NormalsRenderer::fragmentShader = std::string(R"(
#version 430 core

in vec3 c;
out vec4 outColor;

void main()
{
    outColor = vec4(c, 1.0f);
}
)");

#define GROUP_SIZE 128
const std::string NormalsRenderer::generateLineDataShader = std::string(R"(
#version 430 core

// cannot use vec3 for alignment issues
//layout(std430, binding=0) buffer points  { vec3 p[]; };
//layout(std430, binding=1) buffer normals { vec3 n[]; };
//layout(std430, binding=2) buffer lines   { vec3 l[]; };
layout(std430, binding=0) buffer points  { float p[]; };
layout(std430, binding=1) buffer normals { float n[]; };
layout(std430, binding=2) buffer lines   { float l[]; };

layout(location=0) uniform uint numPoints;
layout(location=1) uniform bool do_normalize;

#define GROUP_SIZE 128

layout (local_size_x = GROUP_SIZE, local_size_y = 1) in;

void main()
{
    uint idx = gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x;
    for(; idx < numPoints; idx += gl_WorkGroupSize.x * gl_NumWorkGroups.x) {
        uint i = 3*idx;
        vec3 p0 = vec3(p[i], p[i+1], p[i+2]);
        vec3 n0 = vec3(n[i], n[i+1], n[i+2]);
        if(do_normalize) {
            n0 = normalize(n0);
        }
        vec3 p1 = p0 + n0;

        l[2*i]     = p0.x;
        l[2*i + 1] = p0.y;
        l[2*i + 2] = p0.z;
        l[2*i + 3] = p1.x;
        l[2*i + 4] = p1.y;
        l[2*i + 5] = p1.z;
    }
}


)");

NormalsRenderer::Ptr NormalsRenderer::New(const View::Ptr& view,
                                          const Color& color)
{
    return Ptr(new NormalsRenderer(view, color));
}

NormalsRenderer::NormalsRenderer(const View::Ptr& view,
                                 const Color& color) :
    Renderer(vertexShader, fragmentShader, view),
    numPoints_(0),
    displayData_(0),
    generateLineProgram_(create_compute_program(generateLineDataShader)),
    color_(color)
{
}

NormalsRenderer::~NormalsRenderer()
{
    this->delete_data();
}

void NormalsRenderer::allocate_data(size_t numPoints)
{
    if(!displayData_) {
        glGenBuffers(1, &displayData_);
    }
    if(numPoints > numPoints_) {
        glBindBuffer(GL_ARRAY_BUFFER, displayData_);
        // each point of the model needs 2 points to draw the normal
        glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float)*numPoints, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void NormalsRenderer::delete_data()
{
    if(displayData_ > 0) {
        glDeleteBuffers(1, &displayData_);
    }
    displayData_ = 0;
    numPoints_   = 0;
}

void NormalsRenderer::set_normals(size_t numPoints, GLuint points, GLuint normals,
                                  bool normalizeNormals)
{
    this->allocate_data(numPoints);

    glUseProgram(generateLineProgram_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, points);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, normals);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, displayData_);

    glUniform1ui(0, numPoints);
    if(normalizeNormals)
        glUniform1ui(1, 1);
    else
        glUniform1ui(1, 0);

    glDispatchCompute((numPoints / GROUP_SIZE) + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
    
    // legacy (slow) code to upload normals on device
    //glBindBuffer(GL_ARRAY_BUFFER, points);
    //auto p = static_cast<const float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
    //glBindBuffer(GL_ARRAY_BUFFER, normals);
    //auto n = static_cast<const float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
    //glBindBuffer(GL_ARRAY_BUFFER, displayData_);
    //auto l = static_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    //
    //for(int i = 0; i < 3*numPoints; i+=3) {
    //    std::cout << l[2*i]     << " "  << l[2*i + 1] << " "  << l[2*i + 2] << "\n";
    //    std::cout << l[2*i + 3] << " "  << l[2*i + 4] << " "  << l[2*i + 5] << "\n" << std::endl;
    //    l[2*i]     = p[i];
    //    l[2*i + 1] = p[i + 1];
    //    l[2*i + 2] = p[i + 2];
    //    l[2*i + 3] = p[i]     + n[i];
    //    l[2*i + 4] = p[i + 1] + n[i + 1];
    //    l[2*i + 5] = p[i + 2] + n[i + 2];
    //}

    //glUnmapBuffer(GL_ARRAY_BUFFER);
    //glBindBuffer(GL_ARRAY_BUFFER, normals);
    //glUnmapBuffer(GL_ARRAY_BUFFER);
    //glBindBuffer(GL_ARRAY_BUFFER, points);
    //glUnmapBuffer(GL_ARRAY_BUFFER);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    numPoints_ = numPoints;
}

void NormalsRenderer::set_pose(const Pose& pose)
{
    pose_ = pose;
}

void NormalsRenderer::set_color(const Color& color)
{
    color_[0] = std::max(0.0f, std::min(1.0f, color[0]));
    color_[1] = std::max(0.0f, std::min(1.0f, color[1]));
    color_[2] = std::max(0.0f, std::min(1.0f, color[2]));
}

void NormalsRenderer::draw()
{
    if(displayData_ == 0 || numPoints_ == 0)
        return;
    
    glDisable(GL_DEPTH_TEST);
    Mat4 view = view_->view_matrix() * pose_.homogeneous_matrix();

    glUseProgram(renderProgram_);
    
    glBindBuffer(GL_ARRAY_BUFFER, displayData_);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glUniformMatrix4fv(glGetUniformLocation(renderProgram_, "view"),
        1, GL_FALSE, view.data());
    glUniform3fv(glGetUniformLocation(renderProgram_, "color"),
        1, color_.data());

    glDrawArrays(GL_LINES, 0, 2*numPoints_);
    
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);
}

}; //namespace display
}; //namespace rtac
