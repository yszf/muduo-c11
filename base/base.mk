CXXFLAGS = -O0 -g -Wall -I ../../.. -pthread -std=c++11
LDFLAGS = -lpthread
BASE_SRC = ../Thread.cpp ../CurrentThread.cpp ../CountDownLatch.cpp ../Timestamp.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(BASE_SRC) $(filter %.cpp,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core