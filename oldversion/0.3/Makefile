SOURCE  := $(wildcard *.cc)
OBJS    := $(patsubst %.cc,%.o,$(SOURCE))

TARGET  := myserver
CC   := g++
CXXFLAGS = -std=c++11 -pthread -g -Wall -O3
LIB := -lmysqlclient

.PHONY : objs clean veryclean rebuild all
all : $(TARGET)
objs : $(OBJS)
rebuild: veryclean all
clean :
	rm -fr *.o
veryclean : clean
	rm -rf $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LIB)