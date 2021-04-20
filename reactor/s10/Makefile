LIB_SRC = 			\
	Acceptor.cpp	\
	Buffer.cpp		\
	Channel.cpp		\
	EventLoop.cpp	\
	EventLoopThread.cpp		\
	EventLoopThreadPool.cpp	\
	InetAddress.cpp			\
	Poller.cpp		\
	Socket.cpp		\
	SocketsOps.cpp			\
	TcpConnection.cpp		\
	TcpServer.cpp			\
	Timer.cpp		\
	TimerQueue.cpp			\

BINARIES = test10 test11

all: $(BINARIES)

include ../reactor.mk

test10: test10.cpp
test11: test11.cpp