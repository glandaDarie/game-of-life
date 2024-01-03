COMPILER ?= clang++
CXXFLAGS ?= -std=c++17 -Xpreprocessor -fopenmp

ifeq ($(COMPILER), clang++)
	COMPILER = clang++
else ifeq ($(COMPILER), mpic++)
	COMPILER = mpic++
	CXXFLAGS = -std=c++17
endif

CXX = $(COMPILER)

LDFLAGS = -I /usr/local/opt/libomp/include -L /usr/local/opt/libomp/lib -lomp

SRC_DIR = src
OBJS = openMP.cpp $(SRC_DIR)/constants.cpp $(SRC_DIR)/game_helpers.cpp $(SRC_DIR)/logger.cpp

task: $(OBJS)
	$(CXX) $(CXXFLAGS) -o task $(OBJS) $(LDFLAGS)

random_test1: task
	./task -r 100 -c 1000 -g 10000

random_test2: task
	./task -r 10 -c 10 -g 100

random_test3: task
	./task -r 100 -c 100 -g 10000 -s 50

random_test4: task
	./task -r 1000 -c 100 -g 10000 -s 50

random_test5: task
	mpirun -np 2 ./task -r 10 -c 10 -g 10000 -s 50 --algo MPI

random_test6: task
	./task -r 10 -c 10 -g 10000 -s 50 --algo MPI

generic_test1: task
	./task --tl -g 1

generic_test2: task
	./task --tl -g 1000 -c 2

generic_test3: task
	./task --tl --disp -g 30

generic_test4: task
	./task --tl --trace -g 90

generic_test5: task	
	./task --tl --algo MPI

generic_test6: task	
	mpirun -np 2 ./task --tl -g 1 --algo MPI

generic_test7: task	
	mpiexec -n 6  ./task --tl --algo MPI

.PHONY: clean
clean:
	rm -f task
