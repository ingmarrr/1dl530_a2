#
# # example Makefile to build an executable named myprog from myprog.cpp
# #
SORTED=sorted_list
INTEGRAL=integral
PROG=$(INTEGRAL)

# run: build
# 	@./bin/$(PROG) 10 10

# build: $(PROG).cpp
# 	@g++ -std=c++17 -Wall -pthread $(PROG).cpp -o bin/$(PROG)

# clean:
# 	$(RM) bin/$(PROG)

# given the number of threads as the second arg and number of trepezes as the third
run: build
	./$(PROG).exe 90 10000000 

build: $(PROG).cpp
	g++ -std=c++11 -Wall -pthread $(PROG).cpp -o $(PROG)

clean:
	$(RM) $(PROG)
