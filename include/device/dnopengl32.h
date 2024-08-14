#pragma once



#ifdef __cplusplus
extern "C"{
#endif

	// 
	_declspec(dllexport) void uninitialGL();

	_declspec(dllexport) bool initialGL(HWND hwnd, unsigned int uUniIndexEmu, void* myReadPixels);

	_declspec(dllexport) void readPixels(int x, int y, int w, int h, unsigned int format, unsigned int type, void* pixels);

	_declspec(dllexport) void onAudioNotify(char* data, int size);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
	class _declspec(dllexport) IScreenShotClass {
	public:
		virtual ~IScreenShotClass() {}
		virtual void* cap(void) = 0;
		virtual void release(void) = 0;
	};

	extern "C" __declspec(dllexport) IScreenShotClass* CreateScreenShotInstance(unsigned int playeridx, unsigned int playerpid);
#endif

