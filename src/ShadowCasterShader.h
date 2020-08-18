#ifndef Magnum_Examples_Shadows_ShadowCasterShader_h
#define Magnum_Examples_Shadows_ShadowCasterShader_h

#include <Magnum/GL/AbstractShaderProgram.h>

namespace Magnum { namespace Examples {

class ShadowCasterShader: public GL::AbstractShaderProgram {
    public:
        explicit ShadowCasterShader();

        /**
         * @brief Set transformation matrix
         *
         * Matrix that transforms from local model space -> world space ->
         * camera space -> clip coordinates (aka model-view-projection
         * matrix).
         */
        ShadowCasterShader& setTransformationMatrix(const Matrix4& matrix);

    private:
        Int _transformationMatrixUniform;
};

}}

#endif
