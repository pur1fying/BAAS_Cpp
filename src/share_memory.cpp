//
// Created by Deer on 2022/6/27.
//

#include "share_memory.h"

#ifdef __linux__
namespace ShareMem{

    ShareMemory::ShareMemory(key_t share_key){
        CreateShare(share_key);
    }

    ShareMemory::~ShareMemory() {
        cout<<"析构函数执行"<<endl;
        DestroyShare();
    }

    int ShareMemory::CreateShare(key_t share_key) {
        share_memory_key = share_key;
        // 1.创建共享内存
        share_memory_id = shmget(share_memory_key, sizeof(ShareData), 0666 | IPC_CREAT);
        // 2.映射共享内存地址
        share_memory_address = shmat(share_memory_id, (void*)nullptr, 0);//如果创建一个函数每次调用都执行，需要执行完释放一下shmdt
        // 3.将数据结构指向共享内存地址
        p_share_data = (ShareData *)share_memory_address;
        std::cout<<"共享内存地址 ： "<<(int *)(share_memory_address)<<std::endl;
        return 1;
    }

    int ShareMemory::DestroyShare() const {
        shmdt(share_memory_address);    // 断开映射 ，保证下次访问不被占用
        shmctl(share_memory_id, IPC_RMID, nullptr); // 释放共享内存地址
        cout<<"共享内存已经销毁"<<endl;
        return 1;
    }

    int ShareMemory::PutShareBody(u_char *data_ptr, unsigned long data_size) const {
        if(p_share_data->flag == CAN_WRITE){
            if(data_ptr == nullptr){
                std::cout<<"图片数据不存在"<<std::endl;
                return 0;
            }

            p_share_data->data_size = data_size;
            memcpy(p_share_data->data_body, data_ptr, data_size);

            // 共享内存状态修改为可读
            p_share_data->flag = CAN_READ;

            return 1;
        }else{
            return 0;
        }
    }

    int ShareMemory::GetShareBody(ShareData *recv_ptr) const {
        if(p_share_data->flag == CAN_READ){

            memcpy(recv_ptr, p_share_data, p_share_data->data_size);

            p_share_data->flag = CAN_WRITE;

            return 1;
        }else{
            return 0;
        }
    }

    u_char *ShareMemory::GetShareBodyAddress() const {
        if(p_share_data->flag == CAN_READ){
            return (u_char*)p_share_data->data_body;
        }
    }

    int ShareMemory::FlagStatus() const {
        return p_share_data->flag;
    }

    int ShareMemory::SetStatus(int value) const {
        p_share_data->flag =value;
    }

    ShareData *ShareMemory::ShareMemoryPtr() const {
        return p_share_data;
    }

    ShareMemory::ShareMemory(char PyUseOnly) {
        std::cout<<"[C]: 成功加载共享内存动态库"<<PyUseOnly<<std::endl;
    }

    unsigned long ShareMemory::GetShareBodySize() const {
        if (p_share_data->flag == CAN_READ){
            return p_share_data->data_size;
        }else{
            return 0;
        }
    }

    int ShareMemory::SetShareHead(unsigned long rows, unsigned long cols) const {
        p_share_data->rows = rows;
        p_share_data->cols = cols;
        return 0;
    }


}

/*!
 * @brief 按照C语言格式重新打包-python调用
 * @details 按照C语言格式重新打包-python调用
 * 如果调用端开启了线程  这部份无法直接访问到 需要在线程里面重新创建这个类
 * 如果是单纯的C++调用，不需要这个样子封装
 */
extern "C" {

ShareMem::ShareMemory useShare('.');

int create_share(key_t share_key) {
    useShare.CreateShare(share_key);
}

int destroy_share(){
    useShare.DestroyShare();
}

int put_body(u_char *data_ptr, unsigned long data_size) {
    useShare.PutShareBody(data_ptr, data_size);
}

u_char* get_share_body_address(){
    return useShare.GetShareBodyAddress();
}

unsigned long get_share_body_size(){
    return useShare.GetShareBodySize();
}

int flag_status(){
    useShare.FlagStatus();
}

int set_flag_can_read(){
    useShare.SetStatus(CAN_READ);
}

int set_flag_can_write() {
    useShare.SetStatus(CAN_WRITE);
}

unsigned long get_img_rows() {
    return useShare.ShareMemoryPtr()->rows;
}

unsigned long get_img_cols() {
    return useShare.ShareMemoryPtr()->cols;
}

}

#endif  // __linux__

