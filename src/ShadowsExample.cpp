#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Capsule.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/AbstractObject.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Color.h>

#include "ShadowCasterShader.h"
#include "ShadowReceiverShader.h"
#include "ShadowLight.h"
#include "ShadowCasterDrawable.h"
#include "ShadowReceiverDrawable.h"
#include "Types.h"

namespace Magnum { namespace Examples {

constexpr const float MainCameraNear = 0.01f;
constexpr const float MainCameraFar = 100.0f;

using namespace Math::Literals;

class ShadowsExample: public Platform::Application {
    public:
        explicit ShadowsExample(const Arguments& arguments);

    private:
        struct Model {
            GL::Mesh mesh;
            Float radius;
        };

        void drawEvent() override;
        void mousePressEvent(MouseEvent& event) override;
        void mouseReleaseEvent(MouseEvent& event) override;
        void mouseMoveEvent(MouseMoveEvent& event) override;
        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;

        void addModel(const Trade::MeshData& meshData3D);
        Object3D* createSceneObject(Model& model, bool makeCaster, bool makeReceiver);
        void recompileReceiverShader();
        void setShadowMapSize(const Vector2i& shadowMapSize);

        Scene3D _scene;
        SceneGraph::DrawableGroup3D _shadowCasterDrawables;
        SceneGraph::DrawableGroup3D _shadowReceiverDrawables;
        ShadowCasterShader _shadowCasterShader;
        ShadowReceiverShader _shadowReceiverShader{NoCreate};

        Object3D _shadowLightObject;
        Object3D _cameraObject;

        ShadowLight _shadowLight;
        SceneGraph::Camera3D _camera;

        std::vector<Model> _models;

        Vector3 _mainCameraVelocity;

        Float _shadowBias { 0.003f };
        Vector2i _shadowMapSize { 1024, 1024 };
        Int _shadowMapFaceCullMode { 2 };
        bool _shadowStaticAlignment { false };
};

ShadowsExample::ShadowsExample(const Arguments& arguments):
    Platform::Application{ arguments, Configuration{ }.setTitle("Magnum Shadows Example") },
    _shadowLightObject{ &_scene },
    _shadowLight{ _shadowLightObject },
    _cameraObject{ &_scene },
    _camera{ _cameraObject }
{
    _shadowLight.setupShadowmaps(_shadowMapSize);
    _shadowReceiverShader = ShadowReceiverShader{};
    _shadowReceiverShader.setShadowBias(_shadowBias);

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    // Generate all 3d objects that are to be instanced
    // into the scene.
    addModel(Primitives::cubeSolid());
    addModel(Primitives::capsule3DSolid(1, 1, 4, 1.0f));
    addModel(Primitives::capsule3DSolid(6, 1, 9, 1.0f));

    Object3D* ground = createSceneObject(_models[0], false, true);
    ground->setTransformation(Matrix4::scaling({100, 1, 100}));

    for(std::size_t i = 0; i != 200; ++i) {
        Model& model = _models[std::rand()%_models.size()];
        Object3D* object = createSceneObject(model, true, true);
        object->setTransformation(Matrix4::translation({
            std::rand() * 100.0f / RAND_MAX - 50.0f,
            std::rand() * 5.0f   / RAND_MAX,
            std::rand() * 100.0f / RAND_MAX - 50.0f}));
    }

    _camera.setProjectionMatrix(
        Matrix4::perspectiveProjection(
            35.0_degf,
            Vector2{ GL::defaultFramebuffer.viewport().size() }.aspectRatio(),
            MainCameraNear,
            MainCameraFar
        )
    );

    _cameraObject.setTransformation(Matrix4::translation(Vector3::yAxis(3.0f)));

    _shadowLightObject.setTransformation(
        Matrix4::lookAt(
            { 3.0f, 1.0f, 2.0f },
            {},
            Vector3::yAxis()
        )
    );
}

/**
 * @brief Generate geometry for later compilation into meshes
 *
 * These aren't actually drawn, but rather copied
 * and transformed by `createSceneObject`
 *
 */
void ShadowsExample::addModel(const Trade::MeshData& meshData) {
    _models.emplace_back();
    Model& model = _models.back();

    // Compute bounding sphere of model
    Float maxMagnitudeSquared = 0.0f;
    for(Vector3 position: meshData.positions3DAsArray()) {
        Float magnitudeSquared = position.dot();

        if(magnitudeSquared > maxMagnitudeSquared) {
            maxMagnitudeSquared = magnitudeSquared;
        }
    }

    model.radius = std::sqrt(maxMagnitudeSquared);
    model.mesh = MeshTools::compile(MeshTools::compressIndices(meshData));
}

/**
 * @brief Add a caster and/or reciever object to the scene
 *
 * Notice in particular that each "Model" is instantiated twice
 * most of the time. On rare occasions would you need something
 * to receive but not cast (e.g. ground?) and cast but not receive
 * (e.g. light?)
 *
 * Also notice the `radius` attribute. This is what must be used
 * for culling.
 *
 */
Object3D* ShadowsExample::createSceneObject(Model& model, bool makeCaster, bool makeReceiver) {
    auto* object = new Object3D(&_scene);

    if(makeCaster) {
        auto caster = new ShadowCasterDrawable(*object, &_shadowCasterDrawables);
        caster->setShader(_shadowCasterShader);
        caster->setMesh(model.mesh, model.radius);
    }

    if(makeReceiver) {
        auto receiver = new ShadowReceiverDrawable(*object, &_shadowReceiverDrawables);
        receiver->setShader(_shadowReceiverShader);
        receiver->setMesh(model.mesh);
    }

    return object;
}

void ShadowsExample::drawEvent() {
    if(!_mainCameraVelocity.isZero()) {
        Matrix4 transform = _cameraObject.transformation();
        transform.translation() += transform.rotation()*_mainCameraVelocity*0.3f;
        _cameraObject.setTransformation(transform);
        redraw();
    }

    _shadowLight.setTarget({ 3, 2, 3 }, Vector3::zAxis(), _camera);

    /* Create the shadow map textures. */
    GL::Renderer::setFaceCullingMode(GL::Renderer::PolygonFacing::Front);
    {
        _shadowLight.render(_shadowCasterDrawables);
    }
    GL::Renderer::setFaceCullingMode(GL::Renderer::PolygonFacing::Back);

    /* Render the scene */
    GL::Renderer::setClearColor({0.1f, 0.1f, 0.4f, 1.0f});
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    _shadowReceiverShader.setShadowmapMatrix(_shadowLight.layerMatrix())
                         .setShadowmapTexture(_shadowLight.shadowTexture())
                         .setLightDirection(_shadowLightObject.transformation().backward());

    _camera.draw(_shadowReceiverDrawables);

    swapBuffers();
}

void ShadowsExample::mousePressEvent(MouseEvent& event) {
    if(event.button() != MouseEvent::Button::Left) return;
    event.setAccepted();
}

void ShadowsExample::mouseReleaseEvent(MouseEvent& event) {
    event.setAccepted();
    redraw();
}

void ShadowsExample::mouseMoveEvent(MouseMoveEvent& event) {
    if(!(event.buttons() & MouseMoveEvent::Button::Left)) return;

    const Matrix4 transform = _cameraObject.transformation();

    constexpr const Float angleScale = 0.0005f;
    const Float angleX = event.relativePosition().x() * angleScale;
    const Float angleY = event.relativePosition().y() * angleScale;
    if(angleX != 0.0f || angleY != 0.0f) {
        _cameraObject.setTransformation(Matrix4::lookAt(transform.translation(),
            transform.translation() - transform.rotationScaling()*Vector3{-angleX, angleY, 1.0f},
            Vector3::yAxis()));
    }

    event.setAccepted();
    redraw();
}

void ShadowsExample::keyPressEvent(KeyEvent& event) {
    if(event.key() == KeyEvent::Key::Esc) {
        this->exit();

    } else if(event.key() == KeyEvent::Key::Up) {
        _mainCameraVelocity.z() = -1.0f;

    } else if(event.key() == KeyEvent::Key::Down) {
        _mainCameraVelocity.z() = 1.0f;

    } else if(event.key() == KeyEvent::Key::PageUp) {
        _mainCameraVelocity.y() = 1.0f;

    } else if(event.key() == KeyEvent::Key::PageDown) {
        _mainCameraVelocity.y() = -1.0f;

    } else if(event.key() == KeyEvent::Key::Right) {
        _mainCameraVelocity.x() = 1.0f;

    } else if(event.key() == KeyEvent::Key::Left) {
        _mainCameraVelocity.x() = -1.0f;

    } else if(event.key() == KeyEvent::Key::F3) {
        _shadowMapFaceCullMode = (_shadowMapFaceCullMode + 1) % 3;
        Debug() << "Face cull mode:"
                << (_shadowMapFaceCullMode == 0 ? "no cull" : _shadowMapFaceCullMode == 1
                                                ? "cull back" : "cull front");

    } else if(event.key() == KeyEvent::Key::F4) {
        _shadowStaticAlignment = !_shadowStaticAlignment;
        Debug() << "Shadow alignment:"
            << (_shadowStaticAlignment ? "static" : "camera direction");

    } else if(event.key() == KeyEvent::Key::F7) {
        _shadowReceiverShader.setShadowBias(_shadowBias /= 1.125f);
        Debug() << "Shadow bias" << _shadowBias;

    } else if(event.key() == KeyEvent::Key::F8) {
        _shadowReceiverShader.setShadowBias(_shadowBias *= 1.125f);
        Debug() << "Shadow bias" << _shadowBias;

    } else if(event.key() == KeyEvent::Key::F11) {
        setShadowMapSize(_shadowMapSize / 2);

    } else if(event.key() == KeyEvent::Key::F12) {
        setShadowMapSize(_shadowMapSize * 2);

    } else return;

    event.setAccepted();
    redraw();
}

void ShadowsExample::setShadowMapSize(const Vector2i& shadowMapSize) {
    if((shadowMapSize >= Vector2i{1}).all() && (shadowMapSize <= GL::Texture2D::maxSize()).all()) {
        _shadowMapSize = shadowMapSize;
        _shadowLight.setupShadowmaps(_shadowMapSize);
        Debug() << "Shadow map size" << shadowMapSize << "x";
    }
}

void ShadowsExample::recompileReceiverShader() {
    _shadowReceiverShader = ShadowReceiverShader{};
    _shadowReceiverShader.setShadowBias(_shadowBias);
    for(std::size_t i = 0; i != _shadowReceiverDrawables.size(); ++i) {
        auto& drawable = static_cast<ShadowReceiverDrawable&>(_shadowReceiverDrawables[i]);
        drawable.setShader(_shadowReceiverShader);
    }
}

void ShadowsExample::keyReleaseEvent(KeyEvent &event) {
    if(event.key() == KeyEvent::Key::Up || event.key() == KeyEvent::Key::Down) {
        _mainCameraVelocity.z() = 0.0f;

    } else if (event.key() == KeyEvent::Key::PageDown || event.key() == KeyEvent::Key::PageUp) {
        _mainCameraVelocity.y() = 0.0f;

    } else if (event.key() == KeyEvent::Key::Right || event.key() == KeyEvent::Key::Left) {
        _mainCameraVelocity.x() = 0.0f;

    } else return;

    event.setAccepted();
    redraw();
}

}}

MAGNUM_APPLICATION_MAIN(Magnum::Examples::ShadowsExample)
