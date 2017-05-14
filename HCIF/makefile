CC = g++
CFLAGS= -Wall -Wextra -O2 -std=c++11
LDFLAGS = 
EXEC = $(BUILD_DIR)/hcif

SRC =	main.cpp \
	dataset.cpp

OBJ_DIR = ./obj
SRC_DIR = ./src
BUILD_DIR = ./build
INC_DIR = ./include

OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	
	
$(OBJ_DIR)/main.o: $(INC_DIR)/dataset.h
$(OBJ_DIR)/dataset.o: $(INC_DIR)/dataset.h


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJ_DIR)/*.o

mrproper: clean
	rm -f $(EXEC)
	
