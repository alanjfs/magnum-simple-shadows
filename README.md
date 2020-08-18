Simplest possible shadows with Magnum.

![image](https://user-images.githubusercontent.com/2152766/90515693-e8f68e00-e15a-11ea-9cee-37a8b573eaed.png)

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