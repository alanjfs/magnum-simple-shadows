#ifndef Magnum_Examples_Shadows_ShadowReceiverDrawable_h
#define Magnum_Examples_Shadows_ShadowReceiverDrawable_h

#include <Magnum/GL/Mesh.h>
#include <Magnum/SceneGraph/Drawable.h>

namespace Magnum { namespace Examples {

class ShadowReceiverShader;
class ShadowLight;

/** @brief Drawable that should render shadows cast by casters */
class ShadowReceiverDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ShadowReceiverDrawable(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D* drawables);

        void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D& camera) override;

        GL::Mesh& mesh() { return *_mesh; }
        void setMesh(GL::Mesh& mesh) { _mesh = &mesh; }

        void setShader(ShadowReceiverShader& shader) { _shader = &shader; }

    private:
        GL::Mesh* _mesh{};
        ShadowReceiverShader* _shader{};
};

}}

#endif
