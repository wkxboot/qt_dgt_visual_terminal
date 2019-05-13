#ifndef RECV_THREAD_H
#define RECV_THREAD_H
#include "qthread.h"


namespace wkxboot {
class wkxboot_thread :public QThread{
private:
    void run();
};
}




#endif // RECV_THREAD_H
