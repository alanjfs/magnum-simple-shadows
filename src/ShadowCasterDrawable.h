#ifndef Magnum_Examples_Shadows_ShadowCasterDrawable_h
#define Magnum_Examples_Shadows_ShadowCasterDrawable_h

#include <Magnum/GL/Mesh.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Object.h>

namespace Magnum { namespace Examples {

class ShadowCasterShader;

class ShadowCasterDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ShadowCasterDrawable(SceneGraph::AbstractObject3D& parent, SceneGraph::DrawableGroup3D* drawables);

        /** @brief Mesh to use for this drawable and its bounding sphere radius */
        void setMesh(GL::Mesh& mesh, Float radius) {
            _mesh = &mesh;
            _radius = radius;
        }

        void setShader(ShadowCasterShader& shader) {
            _shader = &shader;
        }

        Float radius() const { return _radius; }

        void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& shadowCamera) override;

    private:
        GL::Mesh* _mesh{};
        ShadowCasterShader* _shader{};
        Float _radius;
};

}}

#endif
