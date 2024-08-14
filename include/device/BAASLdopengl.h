//
// Created by pc on 2024/8/14.
//

#ifndef BAAS_DEVICE_BAASLDOPENGL_H_
#define BAAS_DEVICE_BAASLDOPENGL_H_

#include "BAASConnection.h"
#include "dnopengl32.h"


typedef void* (*CreateShotInstance)(unsigned int player_index, unsigned int player_pid);

class BAASLdopengl {
public:
    struct LDPlayerInfo{
        unsigned int index;     // Emulator instance index, start from 0
        char name[64];          // Instance name
        HWND topWnd;            // Handle of top window
        HWND bndWnd;            // Handle of bind window
        bool sysBoot;           // running status 1 is running, 0 is not running
        DWORD playerPid;        // Process ID of emulator -1 is not running
        DWORD vboxPid;          // Process ID of virtual box -1 is not running
        unsigned int width;
        unsigned int height;
        unsigned int dpi;

        LDPlayerInfo() = default;
    };

    static BAASLdopengl* get_instance(BAASConnection* connection);

    static void release(int instance_Id);

    explicit BAASLdopengl(BAASConnection* connection);

    explicit BAASLdopengl(std::string& installPath);

    void screenshot(cv::Mat &image);

    void detect_ldplayer_instance();

    int get_instance_id() const {
        return instance_id;
    }

private:
    LDPlayerInfo emu_info;

    void init_dll();

    BAASLogger* logger;

    std::string ldplayer_install_path;

    int instance_id;

    std::pair<int, int> resolution;

    static std::map<int, BAASLdopengl*> connections;

    HINSTANCE hDllInst;

    CreateShotInstance createShotInstance = nullptr;

    std::string serial;

    IScreenShotClass* shot_instance = nullptr;
};

class LDOpenGLError : public std::exception {
public:
    explicit LDOpenGLError(const std::string& msg) : message(msg) {}

    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};

#endif //BAAS_DEVICE_BAASLDOPENGL_H_
//
//#include <sstream>
//#include <string>
//#include <list>
//
//
//
//
//using std::list;
//using std::string;
//using std::istringstream;
//using namespace std;
//typedef struct {
//    unsigned short type;
//    unsigned int size;
//    unsigned short reserved1;
//    unsigned short reserved2;
//    unsigned int offset;
//}BMP_FILE_HEADER;
//
//
//typedef struct {
//    unsigned int size;
//    int width, height;
//    unsigned short planes;
//    unsigned short bits;
//    unsigned int compression;
//    unsigned int imagesize;
//    int xresolution, yresolution;
//    unsigned int ncolours;
//    unsigned int importantcolours;
//}BMP_FILE_INFO;
//
//
//typedef struct BMP_Struct{
//    int width;
//    int height;
//    unsigned short bits;
//    unsigned char *data;
//}BMP_FILE, *BMP_FILE_PTR;
//
//
//typedef struct _LDLIST2 {
//    unsigned int index;
//    char name[64];
//    HWND topWnd;
//    HWND bndWnd;
//    BOOL sysboot;
//    DWORD playerpid;
//    DWORD vboxpid;
//    unsigned int width;
//    unsigned int height;
//    unsigned int dpi;
//
//    _LDLIST2(): index(0), topWnd(NULL), bndWnd(NULL), sysboot(FALSE), playerpid(0), vboxpid(0), width(0), height(0), dpi(0) {
//        memset(name, 0, sizeof(name));
//    }
//} LDLIST2, *PLDLIST2;
//
//typedef struct _DEBUGEXECTIME {
//    LARGE_INTEGER s, e;
//    string name;
//} DEBUGEXECTIME, *PDEBUGEXECTIME;
//
//
//#define GL_BGR_EXT  0x80E0
//#define GL_BGRA_EXT 0x80E1
//
//typedef void* (*CREATEINSTANCE)(unsigned int playeridx, unsigned int playerpid);
//
//void saveImage2File(const char* szPath, const char* consum, const char* img, int w, int h, unsigned int format);
//string execldcmd(const string& cmdline);
//list<LDLIST2> parselist2(const string& data);
//LDLIST2 findemu(DWORD idx, string lst);
//void startime(DEBUGEXECTIME& obj);
//char*endptime(DEBUGEXECTIME& obj);
//
//
//int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
//{
//    /*must be x86*/ //HMODULE mod = ::LoadLibraryA(R"(D:\work\leidian\LDPlayer9\ldopengl32.dll)");
//    /*must be x64*/ HMODULE mod = ::LoadLibraryA(R"(C:\leidian\LDPlayer9\ldopengl64.dll)");
//    if (mod == NULL)
//        return -1;
//
//    if (fun == NULL)
//        return -1;
//
//    const int playeridx = 6;
//    CREATEINSTANCE fun = (CREATEINSTANCE) ::GetProcAddress(mod, "CreateScreenShotInstance");
//
//    //ldconsole.exe list2
//    string content = execldcmd(R"(C:\leidian\LDPlayer9\ldconsole.exe list2)");
//    LDLIST2 player = findemu(playeridx, content);
//    if (player.playerpid==0 || player.playerpid==-1)
//        return -1;
//
//    IScreenShotClass* res = (IScreenShotClass*) fun(playeridx, player.playerpid);	//keep instance, constructor is init, destructor is free
//    if (res == NULL)
//        return -1;
//
//    for(size_t i=0; i<10; i++)
//    {
//        DEBUGEXECTIME ob;
//        ob.s.QuadPart = 0;
//        ob.e.QuadPart = 0;
//        ob.name = "LDPlayerScreenShot";
//        startime(ob);
//
//        void* pix = res->cap();
//        if (pix == NULL)
//            return -1;
//
//        char* consum = endptime(ob);
//
//        saveImage2File(R"(D:)", consum, (const char*) pix, player.width, player.height, GL_BGR_EXT);
//    }
//
//    res->release();
//
//    ::Sleep(5000);		//wait for log print
//
//    return 0;
//}
//
//void saveImage2File(const char* szPath, const char* consum, const char* img, int w, int h, unsigned int format)
//{
//    string filename(szPath);
//    char buffer[256] = { 0 };
//    SYSTEMTIME localTime;
//    ::GetLocalTime(&localTime);
//    ::sprintf_s(buffer, 255, "%4d%02d%02d-%02d%02d%02d-%03d",
//                localTime.wYear, localTime.wMonth, localTime.wDay,
//                localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);
//    filename.append("\\");
//    filename.append(buffer);
//    filename.append("-");
//    filename.append(consum);
//    filename.append(".bmp");
//
//    BMP_FILE bmp;
//    bmp.width = w;
//    bmp.height = h;
//    if (format == GL_BGRA_EXT) {
//        bmp.bits = 4;
//    } else {
//        bmp.bits = 3;
//    }
//
//    bmp.data = (unsigned char*)img;
//    void write_bmp_file(const char* output_filename, BMP_FILE* bmp);
//    write_bmp_file(filename.c_str(), &bmp);
//}
//
//
//void write_bmp_file(const char* output_filename, BMP_FILE* bmp)
//{
//    BMP_FILE_HEADER bmpFileHeader;
//    BMP_FILE_INFO bmpFileInfo;
//
//    bmpFileHeader.type = 19778; //  0x4D42; //bmp file
//    bmpFileHeader.size = bmp->width * bmp->height * bmp->bits + 54; // 54 = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
//    bmpFileHeader.reserved1 = 0;
//    bmpFileHeader.reserved2 = 0;
//    bmpFileHeader.offset = 54;
//
//    bmpFileInfo.size = 40;
//    bmpFileInfo.width = bmp->width;
//    bmpFileInfo.height = bmp->height;
//    bmpFileInfo.planes = 1;
//    bmpFileInfo.bits = bmp->bits * 8;
//    bmpFileInfo.compression = 0;
//    bmpFileInfo.imagesize = 0;
//    bmpFileInfo.xresolution = 0;
//    bmpFileInfo.yresolution = 0;
//    bmpFileInfo.ncolours = 0;
//    bmpFileInfo.importantcolours = 0;
//
//    FILE* pFile;
//    fopen_s(&pFile, output_filename, "wb");
//    if (pFile == NULL) {
//        return;
//    }
//    int sizeHeader = sizeof(BMP_FILE_HEADER);
//    int sizeInfo = sizeof(BMP_FILE_INFO);
//
//    fwrite(&bmpFileHeader.type, 1, sizeof(unsigned short), pFile);
//    fwrite(&bmpFileHeader.size, 1, sizeof(unsigned int), pFile);
//    fwrite(&bmpFileHeader.reserved1, 1, sizeof(unsigned short), pFile);
//    fwrite(&bmpFileHeader.reserved2, 1, sizeof(unsigned short), pFile);
//    fwrite(&bmpFileHeader.offset, 1, sizeof(unsigned int), pFile);
//    fwrite(&bmpFileInfo, 1, sizeof(BMP_FILE_INFO), pFile);
//
//    if (bmp->bits == 3) {
//        fwrite(bmp->data, 1, bmp->width * bmp->height * bmp->bits, pFile);
//    } else {
//        char* pixelData;
//        int row;
//        int col;
//        int index1;
//        int index2;
//        for (row = 0; row < bmpFileInfo.height; ++row) {
//            index1 = (bmpFileInfo.height - 1 - row) * bmpFileInfo.width;
//            for (col = 0; col < bmpFileInfo.width; ++col) {
//                index2 = index1 + col;
//                index2 *= 4;
//                pixelData = (char*)bmp->data + index2;
//                fwrite(pixelData + 2, 1, sizeof(unsigned char), pFile);
//                fwrite(pixelData + 1, 1, sizeof(unsigned char), pFile);
//                fwrite(pixelData + 0, 1, sizeof(unsigned char), pFile);
//                fwrite(pixelData + 3, 1, sizeof(unsigned char), pFile);
//            }
//        }
//    }
//
//    fclose(pFile);
//}
//
//string execldcmd(const string& cmdline)
//{
//    STARTUPINFOA si;
//    PROCESS_INFORMATION pi;
//    SECURITY_ATTRIBUTES sa;
//    HANDLE hRead, hWrite;
//    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//    sa.lpSecurityDescriptor = NULL;
//    sa.bInheritHandle = TRUE;
//
//    if (!::CreatePipe(&hRead, &hWrite, &sa, 0)) {
//        ::printf("CreatePipe failed.\n");
//        return "";
//    }
//
//    ZeroMemory(&si, sizeof(si));
//    si.cb = sizeof(si);
//    si.hStdError = hWrite;
//    si.hStdOutput = hWrite;
//    si.dwFlags |= STARTF_USESTDHANDLES;
//
//    char* ptr = ::_strdup(cmdline.c_str());
//    if (ptr == NULL) {
//        ::printf("strdup failed.\n");
//        return "";
//    }
//
//    if (!::CreateProcessA(NULL, ptr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
//        ::printf("CreateProcess failed.\n");
//        ::CloseHandle(hWrite);
//        ::CloseHandle(hRead);
//        return "";
//    }
//
//    ::free(ptr);
//
//    ::CloseHandle(hWrite);
//
//    DWORD bytesRead;
//    CHAR buffer[0x1000] = {0};
//
//    if (!::ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL)) {
//        ::printf("ReadFile failed.\n");
//        ::CloseHandle(hRead);
//        return "";
//    }
//
//    string res = string(buffer, bytesRead);
//
//    ::CloseHandle(hRead);
//
//    ::WaitForSingleObject(pi.hProcess, INFINITE);
//
//    ::CloseHandle(pi.hProcess);
//    ::CloseHandle(pi.hThread);
//
//    return res;
//}
//
//list<LDLIST2> parselist2(const string& data)
//{
//    istringstream iss(data);
//    string line;
//
//    list<LDLIST2> res;
//
//    while(std::getline(iss, line))
//    {
//        LDLIST2 tmp;
//        if (::sscanf_s(line.c_str(), "%u,%[^,],%u,%u,%d,%u,%u,%u,%u,%u",
//                       &tmp.index,
//                       tmp.name, (unsigned int) sizeof(tmp.name)-1,
//                       reinterpret_cast<unsigned int*>(&tmp.topWnd),
//                       reinterpret_cast<unsigned int*>(&tmp.bndWnd),
//                       reinterpret_cast<unsigned int*>(&tmp.sysboot),
//                       &tmp.playerpid,
//                       &tmp.vboxpid,
//                       &tmp.width,
//                       &tmp.height,
//                       &tmp.dpi) == 10)
//        {
//            res.push_back(tmp);
//        }
//    }
//
//    return res;
//}
//
//LDLIST2 findemu(DWORD idx, string lst)
//{
//    list<LDLIST2> emu = parselist2(lst);
//
//    for(const auto& it: emu) {
//        if (it.index == idx){
//            cout << "findemu success" << endl;
//            return it;
//        }
//    }
//
//    return LDLIST2();
//}
//
//void startime(DEBUGEXECTIME& obj)
//{
//    ::QueryPerformanceCounter(&obj.s);
//
//    return;
//}
//
//char* endptime(DEBUGEXECTIME& obj)
//{
//    ::QueryPerformanceCounter(&obj.e);
//
//    LARGE_INTEGER frequency;
//    ::QueryPerformanceFrequency(&frequency);
//    double exectime = static_cast<double>(obj.e.QuadPart - obj.s.QuadPart) / frequency.QuadPart;
//    static char output[0x100] = {0};		//static for return
//    ::sprintf_s(output, "%.5f", exectime);
//
//    return output;
//}
//
//int main() {
//    /*must be x86*/ //HMODULE mod = ::LoadLibraryA(R"(D:\work\leidian\LDPlayer9\ldopengl32.dll)");
//    /*must be x64*/ HMODULE mod = ::LoadLibraryA(R"(C:\leidian\LDPlayer9\ldopengl64.dll)");
//    if (mod == NULL){
//        cout << "LoadLibraryA failed" << endl;
//        return -1;
//    }
//
//    CREATEINSTANCE fun = (CREATEINSTANCE) ::GetProcAddress(mod, "CreateScreenShotInstance");
//    if (fun == NULL) {
//        cout << "GetProcAddress failed" << endl;
//        return -1;
//    }
//
//    const int playeridx = 0;
//
//    //ldconsole.exe list2
//    string content = execldcmd(R"(C:\leidian\LDPlayer9\ldconsole.exe list2)");
//    LDLIST2 player = findemu(playeridx, content);
//    if (player.playerpid==0 || player.playerpid==-1) {
//        cout << "findemu failed" << endl;
//        return -1;
//    }
//
//    IScreenShotClass* res = (IScreenShotClass*) fun(playeridx, player.playerpid);	//keep instance, constructor is init, destructor is free
//    if (res == NULL)
//        return -1;
//
//    for(size_t i=0; i<10; i++)
//    {
//        DEBUGEXECTIME ob;
//        ob.s.QuadPart = 0;
//        ob.e.QuadPart = 0;
//        ob.name = "LDPlayerScreenShot";
//        startime(ob);
//
//        long long start = ::GetTickCount64();
//        void* pix = res->cap();
//        if (pix == NULL)
//            return -1;
//        long long end = ::GetTickCount64();
//        cout << "cap time: " << end - start << "ms" << endl;
//        char* consum = endptime(ob);
//
//        saveImage2File(R"(D:)", consum, (const char*) pix, player.width, player.height, GL_BGR_EXT);
//    }
//
//    res->release();
//
//    ::Sleep(5000);		//wait for log print
//
//    return 0;
//};
