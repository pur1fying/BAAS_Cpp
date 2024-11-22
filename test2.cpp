#include <windows.h>
#include <iostream>

int main() {
    // 打开共享内存对象
    char* st = "test";
    HANDLE hMapFile = OpenFileMapping(
            FILE_MAP_READ,          // 读访问
            FALSE,                  // 不继承句柄
            st // 共享内存名称
    );

    if (hMapFile == NULL) {
        std::cerr << "OpenFileMapping failed: " << GetLastError() << std::endl;
        return 1;
    }

    // 将共享内存映射到进程地址空间
    LPVOID pBuf = MapViewOfFile(
            hMapFile,               // 共享内存句柄
            FILE_MAP_READ,          // 只读访问
            0,                      // 文件偏移的高位
            0,                      // 文件偏移的低位
            1280*720*3                     // 映射的大小
    );

    if (pBuf == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return 1;
    }

    // 从共享内存读取数据
    for (int i = 0; i < 100000; i++) {
        std::cout <<(unsigned int)(((char*)pBuf)[i]) << " ";
    }
    // 取消映射并关闭句柄
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);

    return 0;
}
