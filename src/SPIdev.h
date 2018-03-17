/*  SPIdev.h
 *  
 *  Reference: 
 *    github.com/halherta/RaspberryPi-mcp3008Spi
 *
 * */

#ifndef SPIDEV_H
#define SPIDEV_H

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>

class SPIdev {

  public:
    SPIdev();
    SPIdev( std::string devspi, unsigned char spiMode, unsigned int spiSpeed,
                unsigned char spibitsPerWord );
    ~SPIdev();
    int spiWriteRead( unsigned char *data, int length );

  private:
    unsigned char mode;
    unsigned char bitsPerWord;
    unsigned int speed;
    int spifd;

    int spiOpen( std::string devspi );
    int spiClose();

};

#endif
