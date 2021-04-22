LIB_SRC = 			\
	Acceptor.cpp	\
	Buffer.cpp		\
	Channel.cpp		\
	Connector.cpp	\
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

BINARIES = test9 test12

all: $(BINARIES)

include ../reactor.mk

test9: test9.cpp
test12: test12.cpp