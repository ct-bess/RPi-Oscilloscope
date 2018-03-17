/* *  Oscilloscope.cpp
 *      Oscilloscope driven by the Adafruit MCP3008
 *      Adafruit MCP4725 (DAC) and POT were used for testing
 *
 *  USE CHANNEL 7 of the chip!
 *
 *  TODO: Command args for channels
 *        Variable frequency and sampling size
 *        Better waveform prediction
 *        Support for large samples
 *
 *  Vout formula:
 *    Vout = ( a2dVal / 1024 ) * VREF
 *    VREF = 3.3
 *    a2dVal ranges from 1023 to 0
 *
 *  Sampling:
 *    The 10th's place of VIN increments by 0.1 every 31 iterations
 *    Integer division will be used for plotting
 *      Vout / 31 will yeild 0 to 33 to be plotted
 *      Axises will be labeled from 0.0 to 3.3
 *      which relates to 0.0 (gnd) to 3.3 (vdd)
 * 
 *  Reference:
 *    github.com/halherta/RaspberryPi-mcp3008Spi
 *
 * */
#include "mcp3008Spi.h"
#include <time.h>
#include <ncurses.h>
#include <vector>

// Change this to up the sample size
#define SAMPLE_MODULO 79

// A powerful oscilloscope would find the ideal display frequency
#define PERIOD 250000L

using namespace std;

WINDOW* wnd;

// Initializes the "gui" using ncurses
int init() {

  wnd = initscr();
  cbreak();
  clear();
  refresh();
  
  keypad( wnd, 1 );
  nodelay( wnd, 1 );
  curs_set( 0 );

  if ( !has_colors() ) { return -1; }

  start_color();

  init_pair( 1, COLOR_WHITE, COLOR_BLACK );
  init_pair( 2, COLOR_GREEN, COLOR_BLACK );
  init_pair( 3, COLOR_YELLOW, COLOR_BLACK );
  init_pair( 4, COLOR_RED, COLOR_BLACK );

  wbkgd( wnd, COLOR_PAIR( 1 ) );

  return 1;

}

// Draws the waveform based on the SAMPLE_MODULO (frequency)
//   The size of the vectors is based on the SAMPLE_MODULO
void drawFrame( vector<int> &a2dVec, vector<int> &a2dVecPre ) {

  int point = 0;
  unsigned int i = 0;

  attron( COLOR_PAIR( 3 ) );
  attron( A_BOLD );

  // ncurses leaves anything previously drawn

  // Erase the old, previous frame
  if ( !a2dVecPre.empty() ) {

    while ( i < a2dVecPre.size() ) {

      point = a2dVecPre[i] / 31;

      mvaddch( point, i + 7, ' ' );

      ++i;

    }

  }

  point = 0;
  i = 0;

  // Draw the new, current frame
  while ( i < a2dVec.size() ) {

    // Integer division is important here: truncates output to 2 sigfigs
    point = a2dVec[i] / 31;

    //     ( y,     x, char )
    mvaddch( point, i + 7, '@' );

    ++i;

  }

  attroff( COLOR_PAIR( 3 ) );
  attroff( A_BOLD );

  refresh();
  return;

}

// Predicts the type of wave: square, triangle, or sine
void predictWave( vector<int> &a2dVec ) {

  // Count logic high vs logic low vs mystery logic
  int hl = 0;
  int ll = 0;
  int ml = 0;

  unsigned int i = 0;
  while ( i < a2dVec.size() ) {

    // high for 3.3v to 2.2v
    if ( i >= 22 ) {

      ++hl;

    }

    // mystery zone for 2.1v to 1.3v
    else if ( i >= 13 ) {

      ++ml;

    }

    // low for 1.15v to 0.0v
    else {

      ++ll;

    }

    ++i;

  }

  // print on y = 38, x = 12
  int y = 38;
  int x = 11;

  // Square wave
  if ( hl + ll > ml ) {

    mvprintw( y, x, "Square wave" );

  }
  
  // Triangle wave
  else if ( ml > hl + ll ) {

    mvprintw( y, x, "Triangle wave" );

  }

  // Sine wave
  else if ( ml + ml > hl + ll ) {

    mvprintw( y, x, "Sine wave" );

  } 

  else {

    mvprintw( y, x, "Unknown" );

  }

  return;

}

int main( void ) {

  // init ncurses
  int init_status = init();

  if ( init_status < 0 ) {

    perror( "Terminal does not support colors" );
    endwin();
    exit( 1 );

  }

  // init axises:
  // 3.3v | (wave)
  // ...  | (wave)
  // 0.0v | (wave)
  //      | 00000000001111111111222 ... (most significant digit)
  //      | 01234567890123456789012 ... (least significant digit)

  // Y-axis
  attron( COLOR_PAIR( 2 ) );

  int i = 0;
  int j = 33;
  int d1 = 0;
  int d2 = 0;
  while ( i < 34 ) {

    d2 = j % 10;
    d1 = j % 100;
    d1 = d1 - d2;
    d1 = d1 / 10;

    // Convert to ascii
    d1 += 48;
    d2 += 48;

    mvaddch( i, 0, d1 );
    mvaddch( i, 1, '.' );
    mvaddch( i, 2, d2 );
    mvaddch( i, 3, 'v' );
    mvaddch( i, 4, ' ' );
    mvaddch( i, 5, '|' );
    mvaddch( i, 6, ' ' );
    ++i;
    --j;

  }


  // X-Axis
 
  i = 0;
  j = 33;
  d1 = 0;
  d2 = 0;

  mvaddch( j + 1, i + 5, '|' );
  mvaddch( j + 2, i + 5, '|' );

  while ( i < SAMPLE_MODULO ) {

    d2 = i % 10;
    d1 = i % 100;
    d1 = d1 - d2;
    d1 = d1 / 10;

    d1 += 48;
    d2 += 48;

    mvaddch( j + 1, i + 7, d1 );

    if ( d1 > 0 ) {

      mvaddch( j + 2, i + 7, d2 );

    }

    ++i;

  }

  attroff( COLOR_PAIR( 2 ) );

  // Info section:
  attron( A_BOLD );
  printw( "\nY-Axis: Vout (v)\nX-Axis: Time * 0.00025 (s)" );

  attron( COLOR_PAIR( 4 ) );
  printw( "\nWave-type: " );

  attroff( COLOR_PAIR( 4 ) );
  attroff( A_BOLD );

  refresh();

  int in_char = 0; 

  // Get SPIdev input
  mcp3008Spi a2d( "/dev/spidev0.0", SPI_MODE_0, 1e6, 8 );
  int a2dVal = 0;
  int a2dChan = 7;
  unsigned char data[3];
  vector<int> a2dVec;
  vector<int> a2dVecPre;  // Previous a2dVec

  i = 0;

  // event loop
  for (;;) {

    // Get user's input
    in_char = wgetch( wnd );
    
    // Exit requested
    if ( in_char == 'q' ) { 

      break;

    }

    // start bit
    data[0] = 1;
    
    // 2nd byte transmitted = SGL/DIF = 1, D2 = D1 = D0 = 0
    data[1] = 0b10000000 | ( ( ( a2dChan & 7 ) << 4 ) );  
    
    // "don't care" bit
    data[2] = 0;

    a2d.spiWriteRead( data, sizeof( data ) );
    a2dVal = 0;
    a2dVal = 0b1100000000 & ( data[1] << 8 );
    a2dVal |= ( data[2] & 0xff );

    // Add output for sampling
    a2dVec.push_back( a2dVal );

    // Ideal sleep 
    nanosleep( ( const struct timespec[] ){ { 0, PERIOD } }, NULL );

    ++i;

    // Sampling logic: Increase modulo to up the samples
    //   Ideally the modulo is prime and large
    i = i % SAMPLE_MODULO;

    // Draw the frame when our sample is met
    if ( i == 0 ) {

      drawFrame( a2dVec, a2dVecPre );
      predictWave( a2dVec );

      a2dVecPre.clear();
      a2dVecPre = a2dVec;
      a2dVec.clear();

    }

  } // end event loop

  endwin();
  return 0;

}
