#ifndef REQUEST_H
#define REQUEST_H
#include "qbytearray.h"

class request
{
public:
    request();
    QByteArray m_buffer;
    int m_addr;
    int m_type;
    int m_timeout;
};

#endif // REQUEST_H
