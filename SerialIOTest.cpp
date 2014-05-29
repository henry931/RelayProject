/*
 * q2w.c:
 *      Using the Quick 2 wire board for its mcp23017
 *
 * Copyright (c) 2012-2013 Gordon Henderson. <projects@drogon.net>
 ***********************************************************************
 */

#include <stdio.h>
#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>

int main (void)
{
unsigned int microseconds = 500000;

  int i, bit ;

  wiringPiSetup () ;
  mcp23017Setup (100, 0x20) ;
  mcp23017Setup (200, 0x21) ;

  printf ("Raspberry Pi - MCP23017 Test\n") ;

  //for (i = 0 ; i < 10 ; ++i)
  pinMode (100 , OUTPUT) ;
  pinMode (200 , OUTPUT) ;
  pinMode (101 , INPUT) ;

while(1){

if(digitalRead (101) == 1)
{
digitalWrite (100 , 1) ;
digitalWrite (200 , 1) ;
}
else
{
digitalWrite (100 , 0) ;
digitalWrite (200 , 0) ;
}
usleep(1000);
/*digitalWrite (100 , 1) ;
usleep(microseconds);
digitalWrite (200 , 1) ;
usleep(microseconds);
digitalWrite (100 , 0) ;
usleep(microseconds);
digitalWrite (200 , 0) ;
usleep(microseconds);
*/
}

  //pinMode         (100 + 15, INPUT) ;
  //pullUpDnControl (100 + 15, PUD_UP) ;

  /*for (;;)
  {
    for (i = 0 ; i < 1024 ; ++i)
    {
      for (bit = 0 ; bit < 10 ; ++bit)
        digitalWrite (100 + bit, i & (1 << bit)) ;
      delay (5) ;
      while (digitalRead (100 + 15) == 0)
        delay (1) ;
    }
  }*/
  return 0 ;
}
