### Workflow
1. Build binaries
    - platforms : Windows, Linux, MacOS
    - BuildType : Release
    - Target    : BAAS_ocr_server

2. Test Code
    - Use python unittest to test all the api and check response
    - working directory : folder `BAAS_Cpp`
    - python version : 3.9.18
    - command : 
        1. install dependencies
            ```shell
               pip install -r apps/ocr_server/test/requirements.txt
            ```
        2. run test
           ```shell
              python -m unittest discover -s apps/ocr_server/test -p "*.py"
           ```
    If test passed:
3. Push binaries to corresponding repository
    - Windows: https://github.com/pur1fying/WindowsCompiled_baas_ocr_server.git
    - Linux: https://github.com/pur1fying/LinuxCompiled_baas_ocr_server.git
    - MacOS: https://github.com/pur1fying/MacOSCompiled_baas_ocr_server.git
    **note**: Only update_reference changed files so that the ocr_server updater will not copy all the files every time.


### Known Issues
1. MacOS & Linux : python will destroy the shared_memory automatically when code exit
2. api "get_text_boxes" is not implemented
3. update_reference test code 
    - [x] init / release model  
    - [x] start / stop server 
    - [x] ocr 
    - [x] ocr_for_single_line
    - [x] enable / disable thread pool
    - [x] create / release shared memory
    - [x] get_text_boxes
    - [x] multithread request test

### Android Support
1. [OpenCV 4.9.0 android sdk](https://release-assets.githubusercontent.com/github-production-release-asset/5108051/eb6f2dc7-a522-4eec-92e5-264bf23fc9c1?sp=r&sv=2018-11-09&sr=b&spr=https&se=2025-10-18T13%3A44%3A50Z&rscd=attachment%3B+filename%3Dopencv-4.9.0-android-sdk.zip&rsct=application%2Foctet-stream&skoid=96c2d410-5711-43a1-aedd-ab1947aa7ab0&sktid=398a6654-997b-47e9-b12b-9515b896b4de&skt=2025-10-18T12%3A44%3A38Z&ske=2025-10-18T13%3A44%3A50Z&sks=b&skv=2018-11-09&sig=v5fUuxYONOGNRiGhMKbzDbsyTN9gqwvujqprKHdcGSM%3D&jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmVsZWFzZS1hc3NldHMuZ2l0aHVidXNlcmNvbnRlbnQuY29tIiwia2V5Ijoia2V5MSIsImV4cCI6MTc2MDc5NTIxOSwibmJmIjoxNzYwNzkxNjE5LCJwYXRoIjoicmVsZWFzZWFzc2V0cHJvZHVjdGlvbi5ibG9iLmNvcmUud2luZG93cy5uZXQifQ.PfGRGTEr_SkittPDs5rRb5VMfVU_WUwUH498akpzV_I&response-content-disposition=attachment%3B%20filename%3Dopencv-4.9.0-android-sdk.zip&response-content-type=application%2Foctet-stream) 
2. [ONNXRuntime android lib build guide](https://onnxruntime.ai/docs/build/android.html#build-onnx-runtime-for-android)
3. ONNXRuntime Build Tips : 
   - Onnxruntime version : 1.22.1 
   - NDK version >= 26
   - Architectures : arm64-v8a, x86_64, armeabi-v7a, x86

4. build command
You need to replace -DANDROID_NDK_PATH and -DCMAKE_MAKE_PROGRAM with your own path.
```shell
cmake -G "Ninja" ^
      -B cmake-build-Android_NDK ^
      -DANDROID_NDK_PATH="D:\AndroidSDK\ndk\29.0.14206865" ^
      -DANDROID_ABI="arm64-v8a" ^
      -DANDROID_PLATFORM=android-26 ^
      -DBUILD_BAAS_OCR=ON ^
      -DBAAS_OCR_ANDROID_BUILD=ON ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_MAKE_PROGRAM="D:\AndroidSDK\cmake\3.22.1\bin\ninja.exe"
```

```shell
cmake --build cmake-build-Android_NDK --target BAAS_ocr_server -j 30
```