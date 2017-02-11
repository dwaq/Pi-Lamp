// for threads
#include <thread>

// for unix sockets
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

// for standard I/O
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// to communicate with the thread
typedef enum {
    running,
    start,
    stop,
    kill
} SwitchmateThread;

void setStatus(SwitchmateThread status);
SwitchmateThread getStatus(void);

void setSwitchState(int state);
int getSwitchState(void);

void scan_service(void);

// http://thispointer.com/c11-multithreading-part-2-joining-and-detaching-threads/
class ThreadRAII
{
    std::thread & m_thread;
    public:
        ThreadRAII(std::thread  & thread) : m_thread(thread)
        {

        }

        ~ThreadRAII()
        {
            // Check if thread is joinable then detach the thread
            if(m_thread.joinable())
            {
                m_thread.detach();
            }
        }
};
