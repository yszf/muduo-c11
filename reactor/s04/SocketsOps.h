#ifndef MUDUO_REACTOR_SOCKETSOPS_H
#define MUDUO_REACTOR_SOCKETSOPS_H

#include "muduo-c11/base/Types.h"

#include <endian.h>
#include <netinet/in.h>

namespace muduo {

    namespace sockets {

        inline uint64_t hostToNetwork64(uint64_t host64) {
            return htobe64(host64);
        }

        inline uint64_t hostToNetwork32(uint32_t host32) {
            return htonl(host32);
        }

        inline uint16_t hostToNetwork16(uint16_t host16) {
            return htons(host16);
        }

        inline uint64_t networkToHost64(uint64_t net64) {
            return be64toh(net64);
        }

        inline uint32_t networkToHost32(uint32_t net32) {
            return ntohl(net32);
        }

        inline uint16_t networkToHost16(uint16_t net16) {
            return ntohs(net16);
        }

        int createNonblockingOrDie();

        void bindOrDie(int sockfd, const struct sockaddr_in& addr);

        void listenOrDie(int sockfd);

        int accept(int sockfd, struct sockaddr_in* addr);

        void close(int sockfd);

        void toHostPort(char* buf, size_t size, const struct sockadr_in& adr);

        void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

    } // namespace sockets

} // namespace muduo


#endif // MUDUO_REACTOR_SOCKETSOPS_H