#include<list>
#include<mutex>
#include<thread>
#include<condition_variable>
#include<iostream>
using namespace std;
template<typename T>
class SyncQueue{
public:
    SyncQueue(int maxSize):m_maxSize(maxSize),m_needStop(false)
    {
    }
    //添加事件到列表内，左值拷贝，右值移动
    void Put(const T&x){
        //私有接口
        Add(x);
    }
    void Put(T&&x){
        Add(forward<T>(x));
    }
    //取出列表内的所有事件
    void Take(list<T>&l){
        //当有wait时最好用unique_lock，而不使用lock_guard
        //unique_lock可以在随时释放
        //如果使用lock_guard只会在退出时，释放mutex
        unique_lock<mutex> locker(m_mutex);
        m_notEmpty.wait(locker, [this]{return m_needStop || NotEmpty();});
        if(m_needStop)return;

        l = move(m_queue);
        m_notFull.notify_one();
    }
    //取出一个事件
    void Take(T& t){
        unique_lock<mutex> locker(m_mutex);
        //当线程被唤醒时，且满足两个中任意一条件，就会继续往下运行
        m_notEmpty.wait(locker, [this]{return m_needStop || NotEmpty();});
        //m_needStop为真，表示任务要终止，同样不阻塞
        if(m_needStop)return;

        t=m_queue.front();
        m_queue.pop_front();
        m_notFull.notify_one();
    }
    //结束
    void Stop(){
        {
            lock_guard<mutex>locker(m_mutex);
            m_needStop = true;//将状态置为 true
        }
        //唤醒所有线程
        m_notFull.notify_all();
        m_notEmpty.notify_all();
    }
    //判断队列是否为空
    bool Empty(){
        lock_guard<mutex> locker(m_mutex);
        return m_queue.empty();
    }
    //判断队列是否满
    bool Full(){
        lock_guard<mutex> locker(m_mutex);
        return m_queue.size() == m_maxSize;
    }
    //队列大小
    size_t Size(){
        lock_guard<mutex> locker(m_mutex);
        return m_queue.size();
    }
    //队列大小
    int Count(){
        return m_queue.size();
    }
private:
    //队列未满
    bool NotFull() const{
        bool full = m_queue.size() >= m_maxSize;
        if(full)
            cout<<"缓冲区空了，需要等待。。。，异步层的线程ID："
                <<this_thread::get_id() << endl;
        return !full;
    }
    //队列未空
    bool NotEmpty() const{
        bool empty = m_queue.empty();
        if(empty)
            cout<<"缓冲区空了，需要等待。。。，异步层的线程ID："
                <<this_thread::get_id()<<endl;
        return !empty;
    }
    //向队列中添加事件的实现
    template<typename F>
    void Add(F&&x){
        unique_lock<mutex>locker(m_mutex);
        m_notFull.wait(locker, [this]{return m_needStop || NotFull();});
        if(m_needStop)return;

        m_queue.push_back(forward<F>(x));
        m_notEmpty.notify_one();
    }
private:
    list<T>m_queue;     //缓冲区
    mutex m_mutex;      //互斥量和条件变量结合起来使用
    condition_variable m_notEmpty;      //不为空的条件变量
    condition_variable m_notFull;       //没有满的条件变量
    int m_maxSize;          //同步队列最大的size
    bool m_needStop;        //停止的标志
};

