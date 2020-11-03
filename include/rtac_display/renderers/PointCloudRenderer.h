#ifndef _DEF_RTAC_DISPLAY_POINTCLOUD_RENDERER_H_
#define _DEF_RTAC_DISPLAY_POINTCLOUD_RENDERER_H_

#include <iostream>
#include <array>
#include <algorithm>

#include <rtac_base/types/Handle.h>
#include <rtac_base/types/PointCloud.h>

#include <rtac_display/renderers/Renderer.h>
#include <rtac_display/views/View3D.h>
#include <rtac_display/renderers/NormalsRenderer.h>

namespace rtac { namespace display {


class PointCloudRenderer : public Renderer
{
    public:

    using Ptr      = rtac::types::Handle<PointCloudRenderer>;
    using ConstPtr = rtac::types::Handle<const PointCloudRenderer>;

    using Mat4    = View3D::Mat4;
    using Shape   = View3D::Shape;
    using Pose    = View3D::Pose;
    using Color   = std::array<float,3>;

    static const std::string vertexShader;
    static const std::string fragmentShader;

    protected:
    
    size_t numPoints_;
    GLuint points_;
    Pose   pose_;
    Color  color_;
    NormalsRenderer::Ptr normalsRenderer_;

    void allocate_points(size_t numPoints);
    void delete_points();

    public:
    
    static Ptr New(const View3D::Ptr& view,
                   const Color& color = {0.7,0.7,1.0});
    PointCloudRenderer(const View3D::Ptr& view,
                       const Color& color = {0.7,0.7,1.0});
    ~PointCloudRenderer();
    
    void set_points(size_t numPoints, const float* data);
    void set_points(size_t numPoints, GLuint points);

    // these function use already used points.
    // a call to set points must have been done beforehand.
    void set_normals(size_t numPoints, const float* data,
                     bool normalizeNormals = true);
    void set_normals(size_t numPoints, GLuint normals,
                     bool normalizeNormals = true);

    template <typename PointCloudT>
    void set_points(const rtac::types::PointCloud<PointCloudT>& pc);
    template <typename BufferT>
    void set_points(const BufferT& buffer);
    template <typename Derived>
    void set_points(const Eigen::DenseBase<Derived>& points);

    void set_pose(const Pose& pose);
    void set_color(const Color& color);

    virtual void draw();
    virtual void set_view(const View::Ptr& view);
};

// implementation
template <typename PointCloudT>
void PointCloudRenderer::set_points(const rtac::types::PointCloud<PointCloudT>& pc)
{
    this->allocate_points(pc.size());
    glBindBuffer(GL_ARRAY_BUFFER, points_);
    auto deviceData = static_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    int i = 0;
    for(auto& point : pc) {
        deviceData[i]     = point.x;
        deviceData[i + 1] = point.y;
        deviceData[i + 2] = point.z;
        i += 3;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    numPoints_ = pc.size();
}

template <typename BufferT>
void PointCloudRenderer::set_points(const BufferT& buffer)
{
    this->set_points(buffer->shape().area(), buffer->gl_id());
}

template <typename Derived>
void PointCloudRenderer::set_points(const Eigen::DenseBase<Derived>& points)
{
    //expects points on rows.
    if(points.cols() != 3) {
        throw std::runtime_error("PointCloudRenderer.set_points : Wrong matrix shape");
    }
    size_t numPoints = points.rows();

    this->allocate_points(numPoints);
    glBindBuffer(GL_ARRAY_BUFFER, points_);
    auto deviceData = static_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    for(int i = 0; i < numPoints; i++) {
        deviceData[3*i]     = points(i,0);
        deviceData[3*i + 1] = points(i,1);
        deviceData[3*i + 2] = points(i,2);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    numPoints_ = numPoints;
}

}; //namespace display
}; //namespace rtac

#endif //_DEF_RTAC_DISPLAY_POINTCLOUD_RENDERER_H_