CFLAGS = -Wall -lncurses

all:
	mkdir -p bin
	g++ -o bin/out src/mcp3008Spi.cpp src/Oscilloscope.cpp $(CFLAGS)

clean:
	rm -r bin
