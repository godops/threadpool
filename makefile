test: test.o
	g++ -o test test.o -std=c++11 -lpthread
test.o: test.cpp threadpool.h syncqueue.h
	g++ -c test.cpp -std=c++11
.PHONY:
	clean
clean: 
	rm test.o test
