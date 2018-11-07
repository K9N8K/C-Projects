// **** Include libraries here ****
#include "Field.h"
#include "Protocol.h"
#include "Agent.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries


// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****


// **** Declare any function prototypes here ****

Field test;
int i, j;
uint8_t AStatus, EStatus;

int main()
{
    BOARD_Init();
    printf("test...\n");
    FieldInit(&test, FIELD_POSITION_EMPTY);
    FieldAddBoat(&test, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_SMALL);
    FieldAddBoat(&test, 2, 8, FIELD_BOAT_DIRECTION_WEST, FIELD_BOAT_MEDIUM);
    FieldAddBoat(&test, 5, 7, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_LARGE);
    FieldAddBoat(&test, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_HUGE);
    
    for(i = 0; i < FIELD_ROWS; i++){
        for(j = 0; j < FIELD_COLS; j++){
            printf(" %d ", test.field[i][j]);
        }
        printf("\n");
    }
    
    AStatus = AgentGetStatus();
    EStatus = AgentGetEnemyStatus();
    printf("\nAStatus: %d\n", AStatus);
    printf("\nEStatus: %d\n", EStatus);

    AgentInit();
    
    while (1);
}