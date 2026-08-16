CXX      := g++
CXXFLAGS := -std=c++17 -Wall
CPPFLAGS := -IAPInt

SRCS     := main.cpp APInt/APInt.cpp
OBJS     := $(SRCS:.cpp=.o)
TARGET   := apint_demo

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.cpp APInt/APInt.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

main.o: main.cpp APInt/APInt.h

clean:
	rm -f $(OBJS) $(TARGET)
