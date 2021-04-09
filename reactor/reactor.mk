CXXFLAGS = -O0 -g -Wall -I ../../.. -pthread -std=c++11
LDFLAGS = -lpthread
BASE_SRC = ../../base/Thread.cpp ../../base/CurrentThread.cpp ../../base/CountDownLatch.cpp ../../base/Timestamp.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cpp,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core