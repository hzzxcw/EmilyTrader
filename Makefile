CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -O2 -Wall
LDFLAGS = 

ifeq ($(shell uname), Linux)
    LDFLAGS += -lrt -lpthread
endif

all: simulator recorder strategy

simulator: src/simulator.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

recorder: src/recorder.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

orderbook.o: src/orderbook.cpp include/orderbook.h
	$(CXX) $(CXXFLAGS) -c src/orderbook.cpp -o orderbook.o

strategy: src/strategy.cpp orderbook.o
	$(CXX) $(CXXFLAGS) src/strategy.cpp orderbook.o -o $@ $(LDFLAGS)

clean:
	rm -f simulator recorder strategy orderbook.o market_data.csv
