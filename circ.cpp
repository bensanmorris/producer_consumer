#include <array>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;

int main()
{
    // let's perform this test a bunch of times to make sure we're happy
    for(int i = 0; i < 100; i++)
    {
        const int BUFFER_CAPACITY = 10;
        array<int, BUFFER_CAPACITY> arr;
        int array_size = 0;
    
        condition_variable not_full;
        condition_variable not_empty;
        mutex              mtx;

        // producer
        bool finished = false;
        thread t1([&]() {
            int counter = 0;
            for(int i = 0; i < 20; i++)
            {
                unique_lock<mutex> l(mtx);
                not_full.wait(l, [&](){
                    return array_size < BUFFER_CAPACITY;
                });
                int val = rand() % 255;
                arr[counter] = val;
                cout << "written " << val << endl;
                counter = (counter + 1) % BUFFER_CAPACITY;
                array_size++;
                not_empty.notify_all();
            }
            finished = true;
        });

        // consumer
        thread t2([&](){
            int counter = 0;
            while(true)
            {
                unique_lock<mutex> l(mtx);
                not_empty.wait(l, [&](){
                    return array_size > 0 || finished;
                });

                if(finished && array_size == 0)
                    break;

                int val = arr[counter];
                cout << "read " << val << endl;
                counter = (counter + 1) % BUFFER_CAPACITY;
                array_size--;
                not_full.notify_all();
            }
        });

        t1.join();
        t2.join();
    }
}
