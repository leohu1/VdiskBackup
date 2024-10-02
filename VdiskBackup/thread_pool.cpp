//
// Created by 26071 on 2024/10/1.
//

#include "thread_pool.h"
#include <spdlog/spdlog.h>

namespace thread_pool {
    thread_pool::thread_pool(size_t thead_num) {
        cur_tasks = 0;
        act_threads = thead_num;
        max_threads = thead_num;
        shutdown = false;
        task_list = (task_node *) malloc(sizeof(task_node));
        task_list->next_task = nullptr;
        threads = (std::thread*) malloc(thead_num*sizeof(std::thread));
        if (task_list == nullptr && threads == nullptr){
            SPDLOG_ERROR("malloc error");
            throw std::bad_alloc();
        }
        for (size_t i = 0; i < act_threads; ++i) {
            ::new (threads + i) std::thread(routine, this);
        }
    }
    void thread_pool::routine(thread_pool *t) {
        task_node *p= nullptr;
        while (true){
            std::unique_lock<std::mutex> lock(t->mutex);
            while (t->cur_tasks == 0 && !t->shutdown){
                t->cond.wait(lock);
            }
            if(t->cur_tasks == 0 && t->shutdown)
            {
                t->mutex.unlock();
                return;
            }
            p = t->task_list->next_task;
            //在初始化的时候，第一个任务节点是空的
            t->task_list->next_task = p->next_task;
            p->next_task = nullptr;
            t->cur_tasks--;
            lock.unlock();
            (p->task)(p->arg);
            free(p);
            p = nullptr;
        }
    }
    bool thread_pool::destroy_thread_pool() {
        mutex.lock();
        shutdown = true;
        cond.notify_all();
        mutex.unlock();
        for (size_t i = 0; i < act_threads; ++i){
            threads[i].join();
        }
        free(threads);
        free(task_list);
        return true;
    }
    bool thread_pool::add_task(void (*task)(void *), void *arg) {
        auto *new_task = (task_node*)malloc(sizeof(struct task_node));
        if (new_task == nullptr){
            SPDLOG_ERROR("malloc error");
            throw std::bad_alloc();
        }

        //给新节点赋值
        new_task->task = task;
        new_task->arg = arg;
        new_task->next_task = nullptr;
        std::unique_lock<std::mutex> lock(mutex);
        task_node *tmp = task_list;
        while(tmp->next_task != nullptr)
        {
            tmp = tmp->next_task;
        }
        tmp->next_task = new_task;
        cur_tasks++;
        lock.unlock();
        cond.notify_one();
        return true;
    }
    size_t thread_pool::add_thread(size_t thead_num) {
        if(thead_num == 0)
        {
            return 0;
        }
        size_t total_threads = act_threads + thead_num;
        size_t i = 0,n = 0;
        for(i = act_threads;i < total_threads;i++)
        {
            ::new (threads + i) std::thread(routine, this);
            n++;
        }
        act_threads += n;
        return n;
    }
    size_t thread_pool::remove_thread(size_t thead_num) {
        if(thead_num == 0)
        {
            return act_threads;
        }
        size_t remaining_threads = act_threads - thead_num;
        //为了缓解硬件资源,并不是说销毁线程池,所以至少保留一个活动线程
        remaining_threads = remaining_threads > 0?remaining_threads:1;
        size_t i = 0;
        for(i = act_threads - 1;i > remaining_threads - 1;i--)//数组下标从0开始
        {
            threads[i].join();
        }
        //最终剩下的线程数量
        act_threads = i+1;		//remaining_threads
        return i+1;
    }
    thread_pool::~thread_pool() {
        destroy_thread_pool();
    }
}// namespace thread_pool