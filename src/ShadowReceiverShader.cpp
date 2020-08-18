#include "ShadowReceiverShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/TextureArray.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Matrix4.h>

namespace Magnum { namespace Examples {

ShadowReceiverShader::ShadowReceiverShader(std::size_t numShadowLevels) {
    MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

    const Utility::Resource rs{"shadow-data"};

    GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
    GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

    std::string preamble = "#define NUM_SHADOW_MAP_LEVELS " + std::to_string(numShadowLevels) + "\n";
    vert.addSource(preamble);
    vert.addSource(rs.get("ShadowReceiver.vert"));
    frag.addSource(preamble);
    frag.addSource(rs.get("ShadowReceiver.frag"));

    CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));

    bindAttributeLocation(Position::Location, "position");
    bindAttributeLocation(Normal::Location, "normal");

    attachShaders({vert, frag});

    CORRADE_INTERNAL_ASSERT_OUTPUT(link());

    _modelMatrixUniform = uniformLocation("modelMatrix");
    _transformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
    _shadowmapMatrixUniform = uniformLocation("shadowmapMatrix");
    _lightDirectionUniform = uniformLocation("lightDirection");
    _shadowBiasUniform = uniformLocation("shadowBias");

    setUniform(uniformLocation("shadowmapTexture"), ShadowmapTextureLayer);
}

ShadowReceiverShader& ShadowReceiverShader::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(_transformationProjectionMatrixUniform, matrix);
    return *this;
}

ShadowReceiverShader& ShadowReceiverShader::setModelMatrix(const Matrix4& matrix) {
    setUniform(_modelMatrixUniform, matrix);
    return *this;
}

ShadowReceiverShader& ShadowReceiverShader::setShadowmapMatrices(const Containers::ArrayView<const Matrix4> matrices) {
    setUniform(_shadowmapMatrixUniform, matrices);
    return *this;
}

ShadowReceiverShader& ShadowReceiverShader::setLightDirection(const Vector3& vector) {
    setUniform(_lightDirectionUniform, vector);
    return *this;
}

ShadowReceiverShader& ShadowReceiverShader::setShadowmapTexture(GL::Texture2DArray& texture) {
    texture.bind(ShadowmapTextureLayer);
    return *this;
}

ShadowReceiverShader& ShadowReceiverShader::setShadowBias(const Float bias) {
    setUniform(_shadowBiasUniform, bias);
    return *this;
}

}}
