/*  mcp3008Spi.cpp
 *
 *  Reference:
 *    github.com/halherta/RaspberryPi-mcp3008Spi
 *
 * */

#include "mcp3008Spi.h"
using namespace std;

int mcp3008Spi::spiOpen( string devspi ) {

  int statusVal = -1;
  this->spifd = open( devspi.c_str(), O_RDWR );

  if ( this->spifd < 0 ) {

    perror( "Could not open SPI device" );
    exit( 1 );

  }

  // --- SPIMode WR --- //
  statusVal = ioctl( this->spifd, SPI_IOC_WR_MODE, &( this->mode ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPIMode WR -> ioctl failed" );
    exit( 1 );

  }

  // --- SPIMode RD --- //
  statusVal = ioctl( this->spifd, SPI_IOC_RD_MODE, &( this->mode ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPIMode RD -> ioctl failed" );
    exit( 1 );

  }

  // --- SPI bitsPerWord WR --- //
  statusVal = ioctl( this->spifd, SPI_IOC_WR_BITS_PER_WORD, &( this->bitsPerWord ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPI bitsPerWord WR -> ioctl failed" );
    exit( 1 );

  }

  // --- SPI bitsPerWord RD --- //
  statusVal = ioctl( this->spifd, SPI_IOC_RD_BITS_PER_WORD, &( this->bitsPerWord ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPI bitsPerWord RD -> ioctl failed" );
    exit( 1 );

  }

  // --- SPI speed WR --- //
  statusVal = ioctl( this->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &( this->speed ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPI speed WR -> ioctl failed" );
    exit( 1 );

  }

  // --- SPI speed RD --- //
  statusVal = ioctl( this->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &( this->speed ) );

  if ( statusVal < 0 ) {

    perror( "Could not set SPI speed RD -> ioctl failed" );
    exit( 1 );

  }

  return statusVal;

}

int mcp3008Spi::spiClose() {

  int statusVal = -1;
  statusVal = close( this->spifd );
  
  if ( statusVal < 0 ) {

    perror( "Could not close SPI device" );
    exit( 1 );

  }

  return statusVal;

}

int mcp3008Spi::spiWriteRead( unsigned char *data, int length ) {

  struct spi_ioc_transfer spi[length];
  int i = 0;
  int retVal = -1;

  // ioctl struct must be zero-ed
  bzero( spi, sizeof spi );

  // 1 spi transfer for/e byte
  while ( i < length ) {

    // transfer from "data"
    spi[i].tx_buf = ( unsigned long )( data + i );

    // receive from "data"
    spi[i].rx_buf = ( unsigned long )( data + i );

    // length
    spi[i].len = sizeof( *( data + i ) );

    spi[i].delay_usecs = 0;

    spi[i].speed_hz = this->speed;

    spi[i].bits_per_word = this->bitsPerWord;

    spi[i].cs_change = 0;

    ++i;

  }

  retVal = ioctl( this->spifd, SPI_IOC_MESSAGE( length ), &spi );

  if ( retVal < 0 ) {

    perror( "Error transmitting spi data -> ioctl" );
    exit( 1 );

  }

  return retVal;

}

// --- Default Constructor --- //
mcp3008Spi::mcp3008Spi() {

  this->mode = SPI_MODE_0;
  this->bitsPerWord = 8;
  this->speed = 1e6;
  this->spifd = -1;
  this->spiOpen( string( "/dev/spidev0.0" ) );

}

// --- Overloaded Constructor --- //
mcp3008Spi::mcp3008Spi( string devspi, unsigned char spiMode, 
                        unsigned int spiSpeed, unsigned char spibitsPerWord ) {

  this->mode = spiMode;
  this->bitsPerWord = spibitsPerWord;
  this->speed = spiSpeed;
  this->spifd = -1;
  this->spiOpen( devspi );

}

// --- Destructor --- //
mcp3008Spi::~mcp3008Spi() {

  this->spiClose();

}
