#
# # example Makefile to build an executable named myprog from myprog.cpp
# #
SORTED=sorted_list
INTEGRAL=integral
SIEVE=sieve
FLAKY=flaky
PROG=$(FLAKY)

# given the number of threads as the second arg and number of trepezes as the third
run: build
	./$(PROG).exe 90 10000000 

build: $(PROG).cpp
	g++ -std=c++11 -Wall -pthread $(PROG).cpp -o $(PROG)
	@./bin/$(PROG) 120

build: $(PROG).cpp
	@g++ -std=c++11 -Wall -pthread $(PROG).cpp -o bin/$(PROG)

bench: 
	@g++ -Wall -std=c++11 -pthread -O3 benchmark_example.cpp -o bin/bench
	@bin/bench 32

clean:
	@rm -rf bin
	@mkdir bin

run-win: build
	./bin/$(PROG).exe 10 10

build-win: $(PROG).cpp
	g++ -std=c++11 -Wall -pthread $(PROG).cpp -o ./bin/$(PROG)


