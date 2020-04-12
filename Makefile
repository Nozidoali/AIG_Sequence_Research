CC = g++
CFLAG = 
TARGET = main

INC_DIR = ./abc/src
LIB_DIR = ./lib
SRC_DIR = ./src
OBJ_DIR = ./obj

INC = -I$(INC_DIR)
LIB =  -pthread -lm -L$(LIB_DIR) -labc -ldl -lreadline
SRC = ${wildcard $(SRC_DIR)/*.cpp}
OBJ = ${patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC)}

.PHONY: all test clean

all: clean init $(TARGET) test

init:
	if [ ! -d "obj" ]; then mkdir obj; fi

toy: clean init $(TARGET)
	./main -i data/epfl/random_control/mem_ctrl.blif 2> data.csv
	python sa.py
	
test:
	sh batch.sh

cec:
	./abc/abc -c "cec benchmarks/adder_depth_2018.blif output.blif"

clean:
	if [ -d "obj" ]; then rm -rf obj; fi
	if [ -d "*.out"]; then rm *.out; fi

$(TARGET) : $(OBJ)
	$(CC) $(CFLAG) $? -o $@ $(LIB) $(INC)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAG) -c $< -o $@ $(LIB) $(INC)