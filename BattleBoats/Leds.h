/* 
 * File:   Leds.h
 * Author: kyleko
 *
 * Created on May 15, 2018, 1:56 PM
 */

// Protects from multiple inclusions
#ifndef LEDS_H
#define	LEDS_H

// Local
#include "BOARD.h"

#define LEDS_INIT()\
    do{\
      TRISE = 0;\
      LATE = 0;\
      } while(0)

// This gets which PIN/LED you are focusing on          
#define LEDS_GET() ((LATE) & 0xFF)

#define LEDS_SET(x)  (LATE = x)

#endif