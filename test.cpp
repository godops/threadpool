// File Name: test.cpp

#include<vector>
#include<algorithm>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"threadpool.h"
#include<thread>
using namespace std;

int main(){
    //创建一个对象
    ThreadPool pool;
    //创建线程，向队列内添加一定量任务
    thread thd1([&pool]{
            for(int i=0;i<10;i++){
                    auto thdId = this_thread::get_id();
                    pool.AddTask([thdId]{
                            cout<<"同步层线程1的线程ID："<<thdId<<endl;
                    });
            }
    });

    thread thd2([&pool]{
            for(int i=0;i<10;i++){
                    auto thdId = this_thread::get_id();
                    pool.AddTask([thdId]{
                            cout<<"同步层线程2的线程ID："<<thdId<<endl;
                    });
            }
    });

    this_thread::sleep_for(chrono::seconds(2));
    getchar();
    pool.Stop();
    thd1.join();
    thd2.join();
    return 0;
}
