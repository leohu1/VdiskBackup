//
// Created by 26071 on 2024/10/1.
//

#ifndef VDISKBACKUP_THREAD_POOL_H
#define VDISKBACKUP_THREAD_POOL_H

#include <mutex>

namespace thread_pool {
    struct task_node {
        void (*task)(void *arg);
        void *arg;
        task_node *next_task;
    };
class thread_pool{
private:
    std::mutex mutex;
    std::condition_variable cond;
    std::thread *threads;
    task_node *task_list;
    size_t cur_tasks;
    size_t act_threads;
    size_t max_threads;
    bool shutdown;
    static void routine(thread_pool *t);
public:
    explicit thread_pool(size_t thead_num);
    ~thread_pool();
    bool destroy_thread_pool();
    bool add_task(void (*task)(void *arg), void *arg);
    size_t add_thread(size_t thead_num);
    size_t remove_thread(size_t thead_num);
};

}// namespace thread_pool

#endif//VDISKBACKUP_THREAD_POOL_H
