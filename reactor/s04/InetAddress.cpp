#include "InetAddress.h"

#include <string.h>

using namespace muduo;


InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr;

}