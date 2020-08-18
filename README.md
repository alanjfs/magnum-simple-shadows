Simplest possible shadows with Magnum.

### Build

```bash
git clone --recursive https://github.com/alanjfs/magnum-simple-shadows.git
cd magnum-simple-shadows
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./Release/bin/magnum-simple-shadows
```