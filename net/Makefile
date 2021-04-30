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
	Poller/DefaultPoller.cpp	\
	Poller/EPollPoller.cpp		\
	Poller/PollPoller.cpp		\
	Socket.cpp		\
	SocketsOps.cpp			\
	TcpClient.cpp			\
	TcpConnection.cpp		\
	TcpServer.cpp			\
	Timer.cpp		\
	TimerQueue.cpp			\

BINARIES = tests/EventLoop_unittest

all: $(BINARIES)

include ../net.mk

tests/EventLoop_unittest: tests/EventLoop_unittest.cpp