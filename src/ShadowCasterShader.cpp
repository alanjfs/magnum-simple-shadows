#include "ShadowCasterShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Matrix4.h>

namespace Magnum { namespace Examples {

ShadowCasterShader::ShadowCasterShader() {
    MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

    const Utility::Resource rs{"shadow-data"};

    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};

    vert.addSource(rs.get("ShadowCaster.vert"));
    frag.addSource(rs.get("ShadowCaster.frag"));

    CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));

    attachShaders({vert, frag});

    CORRADE_INTERNAL_ASSERT_OUTPUT(link());

    _transformationMatrixUniform = uniformLocation("transformationMatrix");
}

ShadowCasterShader& ShadowCasterShader::setTransformationMatrix(const Matrix4& matrix) {
    setUniform(_transformationMatrixUniform, matrix);
    return *this;
}

}}
