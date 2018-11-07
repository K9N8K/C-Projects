// **** Include libraries here ****
// Standard libraries
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

//CMPE13 Support Library
#include "BOARD.h"
#include "Field.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries


// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****


// **** Declare any function prototypes here ****

static Field test;
int i, j;

int main()
{
    BOARD_Init();
    FieldInit(&test, FIELD_POSITION_EMPTY);
    FieldAddBoat(&test, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_SMALL);
    FieldAddBoat(&test, 4, 2, FIELD_BOAT_DIRECTION_NORTH, FIELD_BOAT_MEDIUM);
    FieldAddBoat(&test, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_HUGE);
    
    for(i = 0; i < FIELD_ROWS; i++){
        for(j = 0; j < FIELD_COLS; j++){
            printf(" %d", test.field[i][j]);
        }
        printf("\n");
    }
    
    while (1);
}