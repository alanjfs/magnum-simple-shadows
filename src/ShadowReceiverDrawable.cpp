#include "ShadowReceiverDrawable.h"

#include <Corrade/Containers/Array.h>

#include "ShadowReceiverShader.h"
#include "ShadowLight.h"

namespace Magnum { namespace Examples {

ShadowReceiverDrawable::ShadowReceiverDrawable(SceneGraph::AbstractObject3D &object, SceneGraph::DrawableGroup3D* drawables): Drawable{object, drawables} {}

void ShadowReceiverDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) {
    (*_shader)
        .setTransformationProjectionMatrix(camera.projectionMatrix()*transformationMatrix)
        .setModelMatrix(object().transformationMatrix())
        .draw(*_mesh);
}

}}
