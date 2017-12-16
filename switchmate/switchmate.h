#ifndef __SWITCHMATE_H__
#define __SWITCHMATE_H__

// for threads
#include <thread>

// for standard I/O
#include <iostream>

// for bluetooth library
#include <unistd.h>
#include <sys/ioctl.h>

// for bluetooth
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// from bluetooth library
#define HCI_STATE_NONE       0
#define HCI_STATE_OPEN       2
#define HCI_STATE_SCANNING   3
#define HCI_STATE_FILTERING  4

#define EIR_FLAGS                   0X01
#define EIR_NAME_SHORT              0x08
#define EIR_NAME_COMPLETE           0x09
#define SERVICE_DATA                0x16
#define EIR_MANUFACTURE_SPECIFIC    0xFF

void setSwitchState(int state);
int getSwitchState(void);

void printSwitchmateData(void);

void cancelScan(void);

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

#endif
