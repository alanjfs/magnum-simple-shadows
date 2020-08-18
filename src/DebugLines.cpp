#include "DebugLines.h"

#include <Corrade/Containers/ArrayViewStl.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/SceneGraph/Camera.h>

#include "ShadowLight.h"

namespace Magnum { namespace Examples {

DebugLines::DebugLines(): _mesh{GL::MeshPrimitive::Lines} {
    _mesh.addVertexBuffer(_buffer, 0,
        Shaders::VertexColor3D::Position{},
        Shaders::VertexColor3D::Color3{});
}

void DebugLines::reset() {
    _lines.clear();
    _buffer.invalidateData();
}

void DebugLines::addLine(const Point& p0, const Point& p1) {
    _lines.push_back(p0);
    _lines.push_back(p1);
}

bool DebugLines::empty() { return _lines.empty(); }

void DebugLines::addLine(const Vector3& p0, const Vector3& p1, const Color3& col) {
    addLine({p0, col}, {p1, col});
}

void DebugLines::draw(const Matrix4& transformationProjectionMatrix) {
    if(_lines.empty()) return;

    GL::Renderer::disable(GL::Renderer::Feature::DepthTest); {
        _buffer.setData(_lines, GL::BufferUsage::StreamDraw);
        _mesh.setCount(_lines.size());
        _shader
            .setTransformationProjectionMatrix(transformationProjectionMatrix)
            .draw(_mesh);
    }
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
}

void DebugLines::addFrustum(const Matrix4& imvp, const Color3& col) {
    addFrustum(imvp, col, 1.0f, -1.0f);
}

void DebugLines::addFrustum(const Matrix4& imvp, const Color3& col, const Float z0, const Float z1) {
    auto worldPointsToCover = ShadowLight::frustumCorners(imvp, z0, z1);

    auto nearMid = (worldPointsToCover[0] +
                    worldPointsToCover[1] +
                    worldPointsToCover[3] +
                    worldPointsToCover[2])*0.25f;

    addLine(nearMid, worldPointsToCover[1], col);
    addLine(nearMid, worldPointsToCover[3], col);
    addLine(nearMid, worldPointsToCover[2], col);
    addLine(nearMid, worldPointsToCover[0], col);

    addLine(worldPointsToCover[0], worldPointsToCover[1], col);
    addLine(worldPointsToCover[1], worldPointsToCover[3], col);
    addLine(worldPointsToCover[3], worldPointsToCover[2], col);
    addLine(worldPointsToCover[2], worldPointsToCover[0], col);

    addLine(worldPointsToCover[0], worldPointsToCover[4], col);
    addLine(worldPointsToCover[1], worldPointsToCover[5], col);
    addLine(worldPointsToCover[2], worldPointsToCover[6], col);
    addLine(worldPointsToCover[3], worldPointsToCover[7], col);

    addLine(worldPointsToCover[4], worldPointsToCover[5], col);
    addLine(worldPointsToCover[5], worldPointsToCover[7], col);
    addLine(worldPointsToCover[7], worldPointsToCover[6], col);
    addLine(worldPointsToCover[6], worldPointsToCover[4], col);
}

}}
