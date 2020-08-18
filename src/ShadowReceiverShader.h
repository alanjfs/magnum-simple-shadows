#ifndef Magnum_Examples_Shadows_ShadowReceiverShader_h
#define Magnum_Examples_Shadows_ShadowReceiverShader_h

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/Generic.h>

namespace Magnum { namespace Examples {

/** @brief Shader that can synthesize shadows on an object */
class ShadowReceiverShader: public GL::AbstractShaderProgram {
    public:
        typedef Shaders::Generic3D::Position Position;
        typedef Shaders::Generic3D::Normal Normal;

        explicit ShadowReceiverShader(NoCreateT): GL::AbstractShaderProgram{NoCreate} {}

        explicit ShadowReceiverShader();

        /**
         * @brief Set transformation and projection matrix
         *
         * Matrix that transforms from local model space -> world space ->
         * camera space -> clip coordinates (aka model-view-projection matrix).
         */
        ShadowReceiverShader& setTransformationProjectionMatrix(const Matrix4& matrix);

        /**
         * @brief Set model matrix
         *
         * Matrix that transforms from local model space -> world space (used
         * for lighting).
         */
        ShadowReceiverShader& setModelMatrix(const Matrix4& matrix);

        /**
         * @brief Set shadowmap matrix
         *
         * Matrix that transforms from world space -> shadow texture space.
         */
        ShadowReceiverShader& setShadowmapMatrix(const Matrix4 matrix);

        /** @brief Set world-space direction to the light source */
        ShadowReceiverShader& setLightDirection(const Vector3& vector3);

        /** @brief Set shadow map texture array */
        ShadowReceiverShader& setShadowmapTexture(GL::Texture2D& texture);

        /**
         * @brief Set thadow bias uniform
         *
         * Normally it wants to be something from 0.0001 -> 0.001.
         */
        ShadowReceiverShader& setShadowBias(Float bias);

    private:
        enum: Int { ShadowmapTextureLayer = 0 };

        Int _modelMatrixUniform,
            _transformationProjectionMatrixUniform,
            _shadowmapMatrixUniform,
            _lightDirectionUniform,
            _shadowBiasUniform;
};

}}

#endif
