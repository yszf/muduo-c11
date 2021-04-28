#include "SocketsOps.h"

#include <fcntl.h>
#include <assert.h>

using namespace muduo;
using namespace muduo::net;

namespace {

    typedef struct sockaddr SA;

    
    void setNonBlockAndCloseOnExec(int sockfd) {
        // non-block
        int flags = ::fcntl(sockfd, F_GETFL,0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);
        assert(0 == ret);

        // close-on-exec
        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
        assert(0 == ret);
    }

} // namespace

