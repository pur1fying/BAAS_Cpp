#pragma once

#ifdef LDOPENGL_EXPORTS
#define LDOPENGLAPI __declspec(dllexport)
#else
#define LDOPENGLAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
LDOPENGLAPI void uninitialGL();

LDOPENGLAPI bool initialGL(
        HWND hwnd,
        unsigned int uUniIndexEmu,
        void *myReadPixels
);

LDOPENGLAPI void readPixels(
        int x,
        int y,
        int w,
        int h,
        unsigned int format,
        unsigned int type,
        void *pixels
);

LDOPENGLAPI void onAudioNotify(
        char *data,
        int size
);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

class LDOPENGLAPI IScreenShotClass {
public:
    virtual ~IScreenShotClass() {}

    virtual void *cap(void) = 0;

    virtual void release(void) = 0;
};

extern "C" LDOPENGLAPI IScreenShotClass *CreateScreenShotInstance(
        unsigned int playeridx,
        unsigned int playerpid
);
#endif

