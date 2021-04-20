LIB_SRC = 			\
	Acceptor.cpp	\
	Buffer.cpp		\
	Channel.cpp		\
	EventLoop.cpp	\
	EventLoopThread.cpp		\
	InetAddress.cpp			\
	Poller.cpp		\
	Socket.cpp		\
	SocketsOps.cpp			\
	TcpConnection.cpp		\
	TcpServer.cpp			\
	Timer.cpp		\
	TimerQueue.cpp			\

BINARIES = test3 test7 test8

all: $(BINARIES)

include ../reactor.mk

test3: test3.cpp
test7: test7.cpp
test8: test8.cpp