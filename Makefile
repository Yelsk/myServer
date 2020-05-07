CC = g++
CFLAGS = -g -O -Wall

.PHONY:clean

run: HttpConnection.o MutexLock.o Semaphore.o Condition.o run.o
				$(CC) $^ -o $@
%.o:%.cc
				$(CC) $(CFLAGS) -c $^

clean:
				-rm -rf *.o