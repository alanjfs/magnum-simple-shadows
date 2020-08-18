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
        
        // Required, else OpenGL will get mad at you, saying..
        //    Program undefined behavior warning: 
        //    Sampler object 0 does not have depth compare enabled.
        //    It is being used with depth texture 2, by a program
        //    that samples it with a shadow sampler.
        //    This is undefined behavior.
        .setCompareFunction(GL::SamplerCompareFunction::LessOrEqual)
        .setCompareMode(GL::SamplerCompareMode::CompareRefToTexture)
    ;

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

std::vector<Vector4> ShadowLight::calculateClipPlanes() {
    const Matrix4 pm = projectionMatrix();

    // What on earth is happening here?
    std::vector<Vector4> clipPlanes{
        { pm[3][0] + pm[2][0], pm[3][1] + pm[2][1], pm[3][2] + pm[2][2], pm[3][3] + pm[2][3] },   /* near */
        { pm[3][0] - pm[2][0], pm[3][1] - pm[2][1], pm[3][2] - pm[2][2], pm[3][3] - pm[2][3] },   /* far */
        { pm[3][0] + pm[0][0], pm[3][1] + pm[0][1], pm[3][2] + pm[0][2], pm[3][3] + pm[0][3] },   /* left */
        { pm[3][0] - pm[0][0], pm[3][1] - pm[0][1], pm[3][2] - pm[0][2], pm[3][3] - pm[0][3] },   /* right */
        { pm[3][0] + pm[1][0], pm[3][1] + pm[1][1], pm[3][2] + pm[1][2], pm[3][3] + pm[1][3] },   /* bottom */
        { pm[3][0] - pm[1][0], pm[3][1] - pm[1][1], pm[3][2] - pm[1][2], pm[3][3] - pm[1][3] }};  /* top */
    
    for (Vector4& plane : clipPlanes) {
        plane *= plane.xyz().lengthInverted();
    }

    return clipPlanes;
}


void ShadowLight::render(SceneGraph::DrawableGroup3D& drawables) {
    /* Compute transformations of all objects in the group relative to the camera */
    std::vector<std::reference_wrapper<Object3D>> objects;
    objects.reserve(drawables.size());

    for(std::size_t i = 0; i != drawables.size(); ++i) {
        objects.push_back(static_cast<Object3D&>(drawables[i].object()));
    }

    std::vector<ShadowCasterDrawable*> filteredDrawables;

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

    const std::vector<Vector4> clipPlanes = calculateClipPlanes();
    Scene3D* scene = _object.scene();  // Basically the root, top-level object
    std::vector<Matrix4> transformations = scene->transformationMatrices(objects, cameraMatrix());

    /* Rebuild the list of objects we will draw by clipping them with the
       shadow camera's planes */
    std::size_t transformationsOutIndex = 0;
    filteredDrawables.clear();
    for (std::size_t drawableIndex = 0; drawableIndex != drawables.size(); ++drawableIndex) {
        auto& drawable = static_cast<ShadowCasterDrawable&>(drawables[drawableIndex]);
        
        auto& obj = static_cast<Object3D&>(drawables[drawableIndex].object());
        const Matrix4 transform = cameraMatrix()
                                * scene->transformation()
                                * obj.transformation();
        // const Matrix4 transform = transformations[drawableIndex];

        /* If your centre is offset, inject it here */
        const Vector4 localCentre{ 0.0f, 0.0f, 0.0f, 1.0f };
        const Vector4 drawableCentre = transform * localCentre;

        /* Start at 1, not 0 to skip out the near plane because we need to
           include shadow casters traveling the direction the camera is
           facing. */
        bool visible { true };
        for (std::size_t clipPlaneIndex=1; clipPlaneIndex!=clipPlanes.size(); ++clipPlaneIndex) {
            const Float distance = Math::dot(clipPlanes[clipPlaneIndex], drawableCentre);

            // If the object is on the useless side of any one plane, we can skip it 
            if (distance < -drawable.radius()) {
                visible = false;
                break;
            }
        }

        if (visible) {
            /* If this object extends in front of the near plane, extend
               the near plane. We negate the z because the negative z is
               forward away from the camera, but the near/far planes are
               measured forwards. */
            const Float nearestPoint = -drawableCentre.z() - drawable.radius();
            orthographicNear = Math::min(orthographicNear, nearestPoint);
            filteredDrawables.push_back(&drawable);
            transformations[transformationsOutIndex] = transform;
            transformationsOutIndex++;
        }
    }

    /* Recalculate the projection matrix with new near plane. */
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

    for (std::size_t i=0; i!=transformationsOutIndex; ++i) {
        filteredDrawables[i]->draw(transformations[i], *this);
    }

    GL::defaultFramebuffer.bind();
}

}}
