//
// Created by Deer on 2022/6/27.
//

#ifndef BAAS_SHARE_MEMORY_H_
#define BAAS_SHARE_MEMORY_H_
#ifdef __linux__
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <vector>
#include <string>

#include <unistd.h>
#include <sys/shm.h>
using namespace std;

//修改共享内存数据
#define SHARE_MEMORY_SIZE (640 * 640 * 3 * 16)
// 所有的返回函数必须有返回值 不然调用报错

#define CAN_READ 1 //可读
#define CAN_WRITE 0 //可写
#define PROCESSING 2 //处理中

namespace ShareMem{

    //共享内存-数据结构
    struct ShareData{
        // head - 一些固定不变的常量
        unsigned long rows;   //图像高
        unsigned long cols;   //图像宽

        // body - 需要频繁传输的/大容量的数据
        int flag;                               // 标志位
        unsigned long data_size;                // 数据实际大小，用于确定读取量
        char data_body[SHARE_MEMORY_SIZE];      // 共享内存里的数据体
        long frame;                             //帧数
    };

    class ShareMemory{
    private:
        key_t share_memory_key = 0;     //共享内存地址标识
        int share_memory_id = 0;    // 共享内存id
        void *share_memory_address = nullptr;   // 映射共享内存地址  share_memory_address指针记录了起始地址
        ShareData *p_share_data = nullptr;  // 以ShareData结构体类型-访问共享内存

        //未来加速 可以搞一个图像队列 队列大小3  不停存图，然后挨着丢进共享内存，满了就清除。
        //vector<ShareData> share_data_queue;

    public:

        /*!
         * @brief 构造类并创建共享内存
         * @param key 共享内存地址标识
         */
        explicit ShareMemory(key_t share_key);

        /*!
         * @brief 构造一个空类
         * @details 由于Python使用C extension打包，无法动态创建
         * 含参数的类，因此提供此构造函数，用于创建空类。Python可在
         * 后续调用CreateShare完成共享内存的创建
         */
        explicit ShareMemory(char PyUseOnly);

        ~ShareMemory();

        /*!
         * @brief 创建共享内存
         * @details 由于Python使用C extension打包，无法动态创建
         * 含参数的类，因此提供此方法来手动创建共享内存
         * @param share_key 共享内存地址标识
         * @return
         */
        int CreateShare(key_t share_key);

        /*!
         * @brief 销毁共享内存
         * @return
         */
        int DestroyShare() const;

        /*!
         * @brief 设置共享内存的 head 部分
         * @details head 部分为一些不常修改的数据，比如图像的宽、高等
         * 可以用于包含一些用于告知接收端的"协议性"信息
         * @return
         */
        int SetShareHead(unsigned long rows, unsigned long cols) const;

        /*!
         * @brief 将数据写入共享内存
         * @param data_ptr 要写入的数据的地址
         * @param data_size 要发送的数据大小
         * @return 1成功 0失败
         * @details 传入要写入数据的指针，函数会讲数据拷贝进共享内存，修改同时flag为CAN_READ，允许其他进程调用接收函数接受图像。
         */
        int PutShareBody(uint8_t *data_ptr, unsigned long data_size) const;

        /*!
         * @brief 获取数据体的实际大小
         * @return 数据体的实际大小，失败则返回0
         */
        unsigned long GetShareBodySize() const;

        /*!
         * @brief 从共享内存中拷贝出数据体
         * @details 传入用于接收数据的指针，函数会讲数据拷贝出来，修改同时flag为CAN_WRITE，允许其他进程调用写入函数写入图像。
         * @return
         */
        int GetShareBody(ShareData *recv_ptr) const;

        /*!
         * @brief 获取共享内存数据的指针
         * @details 这个函数仅获取共享内存中data_body的指针，不会改变flag的值。
         * @return 共享内存数据的指针
         */
        uint8_t * GetShareBodyAddress() const;

        /*!
         * @brief 获取共享内存读写状态
         * @details 用于判断是否可以读写
         * @return 共享内存读写状态
         */
        int FlagStatus() const;

        /*!
         * @brief 设置共享内存的读写状态
         * @details CAN_READ （1）可读；CAN_WRITE 可写（0）；
         * @param value 0 为可写，1为可读
         * @return
         */
        int SetStatus(int value) const;

        /*!
         * @brief 获取共享内存的指针
         * @details 该函数可以直接获得共享内存地址，越权操作共享内存里的内容
         * 通常来说不建议使用，因为可能导致内存溢出等灾难性的问题
         * @return
         */
        ShareData *ShareMemoryPtr() const;

    };

}
#endif //__linux__
#endif //BAAS_SHARE_MEMORY_H_
