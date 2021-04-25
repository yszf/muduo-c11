LIB_SRC = 			\
	Acceptor.cpp	\
	Buffer.cpp		\
	Channel.cpp		\
	Connector.cpp	\
	EPoller.cpp		\
	EventLoop.cpp	\
	EventLoopThread.cpp		\
	EventLoopThreadPool.cpp	\
	InetAddress.cpp			\
	Poller.cpp		\
	Socket.cpp		\
	SocketsOps.cpp			\
	TcpClient.cpp			\
	TcpConnection.cpp		\
	TcpServer.cpp			\
	Timer.cpp		\
	TimerQueue.cpp			\

BINARIES = test9 test13

all: $(BINARIES)

include ../reactor.mk

test9: test9.cpp
test13: test13.cpp