param (
    [string]$buildtype = "Release",
    [int]$clean = 0  # Before building, delete the build directory entirely
)

function check_command {
    param([string]$command)
    if ((get-command $command -ea silentlycontinue) -eq $null) {
        write-host "$command not found!"
        return 1;
    }
    return 0;
}

function check_env {
    param([string]$key)
    if ([Environment]::getenvironmentvariable($key) -eq "") {
        write-host "$key unset!"
        return 1;
    }
    return 0;
}

# Check prerequisites
$missing = check_command("cmake")
$missing += check_command("ninja")
$missing += check_command("em++")
$missing += check_command("python")
$missing += check_env("EMSDK")

if ($missing) {
    return;
}

$EMSCRIPTEN_PREFIX=$env:EMSDK.replace("\\", "/") + "/upstream/emscripten"
$builddir = "$psscriptroot/build/emscripten"

if ($clean -ne 0) {
    write-host "-- Cleaning.."
    rm -r -force -ea silentlycontinue $builddir
}

pushd # Store current path

mkdir -ea silentlycontinue $builddir
cd $builddir

cmake ../.. -G Ninja `
    -DCMAKE_BUILD_TYPE="$buildtype" `
    -DPLATFORM="Emscripten" `
    -DLIB_SUFFIX="64" `
    -DCORRADE_RC_EXECUTABLE="$psscriptroot/external/corrade-rc.exe" `
    -DCMAKE_TOOLCHAIN_FILE="$psscriptroot/cmake/toolchains/generic/Emscripten-wasm.cmake" `
    -DCMAKE_INSTALL_PREFIX="$psscriptroot/install"

cmake --build . --config Release --target install

popd # Restore current path
