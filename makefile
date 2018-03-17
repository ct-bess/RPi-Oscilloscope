all: YeET

YeET:
  mkdir -p bin
  g++ -Wall -o outbin src/SPIdev.cpp src/Oscilloscope.cpp -lncurses

clean:
  rm -r bin
