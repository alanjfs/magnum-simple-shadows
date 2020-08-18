#include "ShadowCasterDrawable.h"

#include <Magnum/SceneGraph/Camera.h>

#include "ShadowCasterShader.h"

namespace Magnum { namespace Examples {

ShadowCasterDrawable::ShadowCasterDrawable(SceneGraph::AbstractObject3D& parent, SceneGraph::DrawableGroup3D* drawables): Magnum::SceneGraph::Drawable3D{parent, drawables} {}

void ShadowCasterDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& shadowCamera) {
    (*_shader)
        .setTransformationMatrix(shadowCamera.projectionMatrix()*transformationMatrix)
        .draw(*_mesh);
}

}}
