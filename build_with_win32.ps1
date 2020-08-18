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

# Check prerequisites
$missing = check_command("cmake")
$missing += check_command("ninja")
$missing += check_command("cl")

if ($missing) {
    return;
}

$builddir = "$psscriptroot/build/win32"

if ($clean -ne 0) {
    write-host "-- Cleaning.."
    rm -r -force -ea silentlycontinue $builddir
}

write-host "Building $buildtype to $builddir"
pushd # Store current path

mkdir -ea silentlycontinue $builddir
cd $builddir

cmake ../.. -G Ninja `
    -DCMAKE_BUILD_TYPE="$buildtype" `
    -DCMAKE_INSTALL_PREFIX="$psscriptroot/install" 

cmake --build . --config $buildtype --target install

popd # Restore current path