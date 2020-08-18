#ifndef Magnum_Examples_Shadows_ShadowLight_h
#define Magnum_Examples_Shadows_ShadowLight_h

#include <Magnum/Resource.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/TextureArray.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/AbstractFeature.h>

#include "Types.h"

namespace Magnum { namespace Examples {

/**
 * @brief A special camera used to render shadow maps
 *
 * The object it's attached to should face the direction that the light travels.
 *
*/
class ShadowLight: public SceneGraph::Camera3D {
    public:
        static std::vector<Vector3> cameraFrustumCorners(SceneGraph::Camera3D& mainCamera, Float z0 = -1.0f, Float z1 = 1.0f);

        static std::vector<Vector3> frustumCorners(const Matrix4& imvp, Float z0, Float z1);

        explicit ShadowLight(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& parent);

        /**
         * @brief Initialize the shadow map texture array and framebuffers
         *
         * Should be called before @ref setupSplitDistances().
         *
         */
        void setupShadowmaps(Int numShadowLevels, const Vector2i& size);

        /**
         * @brief Set up the distances we should cut the view frustum along
         *
         * The distances will be distributed along a power series. Should be
         * called after @ref setupShadowmaps().
         */
        void setupSplitDistances(Float cameraNear, Float cameraFar, Float power);

        /**
         * @brief Computes all the matrices for the shadow map splits
         * @param lightDirection    Direction of travel of the light
         * @param screenDirection   Crossed with light direction to determine
         *      orientation of the shadow maps. Use the forward direction of
         *      the camera for best resolution use, or use a constant value for
         *      more stable shadows.
         * @param mainCamera        The camera to use to determine the optimal
         *      splits (normally, the main camera that the shadows will be
         *      rendered to)
         *
         * Should be called whenever your camera moves.
         */
        void setTarget(const Vector3& lightDirection, const Vector3& screenDirection, SceneGraph::Camera3D& mainCamera);

        /**
         * @brief Render a group of shadow-casting drawables to the shadow maps
         */
        void render(SceneGraph::DrawableGroup3D& drawables);

        std::vector<Vector3> layerFrustumCorners(SceneGraph::Camera3D& mainCamera, Int layer);

        Float cutZ(Int layer) const;

        Float cutDistance(Float zNear, Float zFar, Int layer) const;

        std::size_t layerCount() const { return _layers.size(); }

        const Matrix4& layerMatrix(Int layer) const {
            return _layers[layer].shadowMatrix;
        }

        std::vector<Vector4> calculateClipPlanes();

        GL::Texture2DArray& shadowTexture() { return _shadowTexture; }

    private:
        Object3D& _object;
        GL::Texture2DArray _shadowTexture;

        struct ShadowLayerData {
            GL::Framebuffer shadowFramebuffer;
            Matrix4 shadowCameraMatrix;
            Matrix4 shadowMatrix;
            Vector2 orthographicSize;
            Float orthographicNear, orthographicFar;
            Float cutPlane;

            explicit ShadowLayerData(const Vector2i& size);
        };

        std::vector<ShadowLayerData> _layers;
};

}}

#endif
