#include "ShadowLight.h"

#include <algorithm>  // std::any_of
#include <Magnum/ImageView.h>
#include <Magnum/Image.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/SceneGraph/FeatureGroup.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>

#include "ShadowCasterDrawable.h"

namespace Magnum { namespace Examples {

ShadowLight::ShadowLight(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& parent)
    : SceneGraph::Camera3D{parent},
      _object(parent) {
    setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::NotPreserved);
}

void ShadowLight::setupShadowmaps(const Vector2i& size) {
    _shadowTexture = GL::Texture2D{};
    _shadowTexture

        // See also DepthComponent8/16/32
        .setStorage(1, GL::TextureFormat::DepthComponent24, size)
        
        // Required, else OpenGL will tell you..
        //    Program undefined behavior warning: 
        //    Sampler object 0 does not have depth compare enabled.
        //    It is being used with depth texture 2, by a program
        //    that samples it with a shadow sampler.
        //    This is undefined behavior.
        .setCompareFunction(GL::SamplerCompareFunction::LessOrEqual)
        .setCompareMode(GL::SamplerCompareMode::CompareRefToTexture)
    ;

    if (_data != nullptr) delete _data;
    _data = new ShadowData{size};

    GL::Framebuffer& shadowFramebuffer = _data->shadowFramebuffer;
    shadowFramebuffer.attachTexture(GL::Framebuffer::BufferAttachment::Depth,
                                    _shadowTexture, 0)
                     .mapForDraw(GL::Framebuffer::DrawAttachment::None)
                     .bind();

    CORRADE_INTERNAL_ASSERT(
        shadowFramebuffer.checkStatus(GL::FramebufferTarget::Draw) ==
        GL::Framebuffer::Status::Complete
    );
}

ShadowLight::ShadowData::ShadowData(const Vector2i& size) 
    : shadowFramebuffer{ { {}, size } } {}


void ShadowLight::setTarget(const Vector3& lightDirection,
                            const Vector3& screenDirection,
                            SceneGraph::Camera3D& mainCamera) {
    Matrix4 cameraMatrix = Matrix4::lookAt({}, -lightDirection, screenDirection);
    const Matrix3x3 cameraRotationMatrix = cameraMatrix.rotation();
    const Matrix3x3 inverseCameraRotationMatrix = cameraRotationMatrix.inverted();

    std::size_t dataIndex { 0 };

    std::vector<Vector3> mainCameraFrustumCorners = layerFrustumCorners(
        mainCamera, Int(dataIndex)
    );

    /* Calculate the AABB in shadow-camera space */
    Vector3 min { std::numeric_limits<Float>::max() };
    Vector3 max { std::numeric_limits<Float>::lowest() };

    for (Vector3 worldPoint: mainCameraFrustumCorners) {
        Vector3 cameraPoint = inverseCameraRotationMatrix * worldPoint;
        min = Math::min(min, cameraPoint);
        max = Math::max(max, cameraPoint);
    }

    /* Place the shadow camera at the mid-point of the camera box */
    const Vector3 mid = (min + max) * 0.5f;
    const Vector3 cameraPosition = cameraRotationMatrix * mid;
    const Vector3 range = max - min;

    /* Set up the initial extends of the shadow map's render volume. Note
       we will adjust this later when we render. */
    _data->orthographicSize = range.xy();
    _data->orthographicNear = -0.5f * range.z();
    _data->orthographicFar =  0.5f * range.z();
    cameraMatrix.translation() = cameraPosition;
    _data->shadowCameraMatrix = cameraMatrix;
}

std::vector<Vector3> ShadowLight::layerFrustumCorners(SceneGraph::Camera3D& mainCamera,
                                                       const Int data) {
    const Float z0 = data == 0 ? 0 : _data->cutPlane;
    const Float z1 = _data->cutPlane;
    return cameraFrustumCorners(mainCamera, z0, z1);
}

std::vector<Vector3> ShadowLight::cameraFrustumCorners(SceneGraph::Camera3D& mainCamera,
                                                       const Float z0,
                                                       const Float z1) {
    const Matrix4 imvp = (mainCamera.projectionMatrix()*mainCamera.cameraMatrix()).inverted();
    return frustumCorners(imvp, z0, z1);
}

std::vector<Vector3> ShadowLight::frustumCorners(const Matrix4& imvp,
                                                 const Float z0,
                                                 const Float z1) {
    return {imvp.transformPoint({-1,-1, z0}),
            imvp.transformPoint({ 1,-1, z0}),
            imvp.transformPoint({-1, 1, z0}),
            imvp.transformPoint({ 1, 1, z0}),
            imvp.transformPoint({-1,-1, z1}),
            imvp.transformPoint({ 1,-1, z1}),
            imvp.transformPoint({-1, 1, z1}),
            imvp.transformPoint({ 1, 1, z1})};
}


void ShadowLight::render(SceneGraph::DrawableGroup3D& drawables) {
    /* Compute transformations of all objects in the group relative to the camera */
    // std::vector<std::reference_wrapper<Object3D>> objects;
    // objects.reserve(drawables.size());

    // for(std::size_t i = 0; i != drawables.size(); ++i) {
    //     objects.push_back(static_cast<Object3D&>(drawables[i].object()));
    // }

    // std::vector<ShadowCasterDrawable*> filteredDrawables;

    /* Projecting world points normalized device coordinates means they range
       -1 -> 1. Use this bias matrix so we go straight from world -> texture
       space */
    constexpr const Matrix4 bias {{0.5f, 0.0f, 0.0f, 0.0f},
                                  {0.0f, 0.5f, 0.0f, 0.0f},
                                  {0.0f, 0.0f, 0.5f, 0.0f},
                                  {0.5f, 0.5f, 0.5f, 1.0f}};

    GL::Renderer::setDepthMask(true);

    Float orthographicNear = _data->orthographicNear;
    const Float orthographicFar = _data->orthographicFar;

    /* Move this whole object to the right place to render each _data */
    _object.setTransformation(_data->shadowCameraMatrix)
           .setClean();

    setProjectionMatrix(
        Matrix4::orthographicProjection(
            _data->orthographicSize,
            orthographicNear,
            orthographicFar
        )
    );

    const Matrix4 shadowCameraProjectionMatrix = Matrix4::orthographicProjection(
        _data->orthographicSize,
        orthographicNear,
        orthographicFar
    );

    _data->shadowMatrix = bias
                       * shadowCameraProjectionMatrix
                       * cameraMatrix();

    setProjectionMatrix(shadowCameraProjectionMatrix);

    _data->shadowFramebuffer.clear(GL::FramebufferClear::Depth)
                           .bind();

    // for (std::size_t i=0; i!=transformationsOutIndex; ++i) {
    for (std::size_t i = 0; i != drawables.size(); ++i) {
        auto& obj = static_cast<Object3D&>(drawables[i].object());
        auto transform = cameraMatrix()
                       * _object.scene()->transformation()
                       * obj.transformation();
        drawables[i].draw(transform, *this);
    }

    GL::defaultFramebuffer.bind();
}

}}
