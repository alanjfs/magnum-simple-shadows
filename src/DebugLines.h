#ifndef Magnum_Examples_Shadows_DebugLines_h
#define Magnum_Examples_Shadows_DebugLines_h

#include <vector>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/SceneGraph.h>
#include <Magnum/Shaders/VertexColor.h>

namespace Magnum { namespace Examples {

class DebugLines {
    public:
        struct Point {
            Vector3 position;
            Color3 color;
        };

        explicit DebugLines();

        void reset();
        bool empty();

        void addLine(const Point& p0, const Point& p1);
        void addLine(const Vector3& p0, const Vector3& p1, const Color3& col);
        void addFrustum(const Matrix4& imvp, const Color3& col);
        void addFrustum(const Matrix4& imvp, const Color3& col, Float z0, Float z1);

        void draw(const Matrix4& transformationProjectionMatrix);

    protected:
        std::vector<Point> _lines;
        GL::Buffer _buffer;
        GL::Mesh _mesh;
        Shaders::VertexColor3D _shader;
};

}}

#endif
