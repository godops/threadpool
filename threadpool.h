#include<list>
#include<thread>
#include<functional>
#include<memory>
#include<atomic>
#include"syncqueue.h"
//最大的任务数
const int MaxTaskCount = 100;
class ThreadPool{
public:
    using Task = function<void()>;
    ThreadPool(int numThreads = thread::hardware_concurrency())
        :m_queue(MaxTaskCount)
    {
        cout<<numThreads<<endl;
        Start(numThreads);
    }
    ~ThreadPool(void){
        //如果没有停止，则主动停止
        Stop();
    }
    //停止只需一次，所以这里使用call_once
    void Stop(){
        call_once(m_flag, [this]{StopThreadGroup();});
    }
    //添加任务到队列中
    void AddTask(Task&&task){
        m_queue.Put(forward<Task>(task));
    }
    void AddTask(const Task &task){
        m_queue.Put(task);
    }
private:
    void Start(int numThreads){
        m_running = true;
        //创建线程组，这里使用了智能指针
        for(int i=0;i<numThreads; ++i){
            m_threadgroup.push_back(make_shared<thread>
                    (&ThreadPool::RunInThread, this));
        }
    }
    //一次取出队列中的所有事件
    void RunInThread(){
        while(m_running){
            list<Task> l;
            m_queue.Take(l);

            for(auto& task : l){
                if(!m_running)
                    return;

                task();
            }
        }
    }
    //停止线程池
    void StopThreadGroup(){
        m_queue.Stop();
        m_running = false;

        for(auto th : m_threadgroup){
            if(th)th->join();
        }
        m_threadgroup.clear();
    }
private:
    list<shared_ptr<thread>>m_threadgroup;      //处理任务的线程组
    SyncQueue<Task> m_queue;                    //同步队列
    atomic_bool m_running;                      //是否停止的标志
    once_flag m_flag;
};
