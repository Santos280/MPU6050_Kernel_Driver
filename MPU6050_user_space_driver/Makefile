# Define the compiler
CC = gcc

# Define the name of the executable
TARGET = main

# Define the source files
SRCS = main.c mpu6050.c

# Define the object files (automatically derived from source files)
OBJS = $(SRCS:.c=.o)

# Default target: builds the program
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Generic rule to compile a .c file into a .o file
# This tells make how to handle the compilation of each source file
%.o: %.c
	$(CC) -c $< -o $@

# Target to run the compiled program
run: $(TARGET)
	./$(TARGET)

# Target to clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run
