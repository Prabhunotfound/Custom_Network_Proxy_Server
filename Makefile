CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -Iinclude

SRC = src/main.cpp src/client_handler.cpp \
      src/http_parser.cpp src/forwarder.cpp src/logger.cpp \
      src/blocklist.cpp src/thread_pool.cpp src/server.cpp \
	  src/config.cpp src/global_config.cpp src/metrics.cpp

OUT = proxy

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
