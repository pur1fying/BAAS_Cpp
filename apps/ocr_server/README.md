### Workflow
1. Build binaries
    - platforms : Windows, Linux, MacOS
    - BuildType : Release
    - Target    : BAAS_ocr_server

2. Test Code
    - Use python to test all the api and check response
    TODO:: write test script

    If test passed:
3. Push binaries to corresponding repository
    - Windows: https://github.com/pur1fying/WindowsCompiled_baas_ocr_server.git
    - Linux: https://github.com/pur1fying/LinuxCompiled_baas_ocr_server.git
    - MacOS: https://github.com/pur1fying/MacOSCompiled_baas_ocr_server.git
    **note**: Only update changed files so that the ocr_server updater will not copy all the files every time.


### Known Issues
1. MacOS & Linux : python will destroy the shared_memory automatically when code exit
2. api "get_text_boxes" is not implemented
3. need a method / api to shutdown the server    
4. update test code