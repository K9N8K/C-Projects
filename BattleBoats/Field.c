/*
 * File:   AaronField.c
 * Author: Aaron Lethers
 *
 * Created on June 2, 2018, 8:46 PM
 * 
 * Partner: Kyle Ko
 */

#include "Field.h"
#include "Protocol.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * FieldInit() will fill the passed field array with the data specified in positionData. Also the
 * lives for each boat are filled according to the `BoatLives` enum.
 * @param f The field to initialize.
 * @param p The data to initialize the entire field to, should be a member of enum
 *                     FieldPosition.
 * typedef enum {
    FIELD_BOAT_LIVES_SMALL  = 3,
    FIELD_BOAT_LIVES_MEDIUM = 4,
    FIELD_BOAT_LIVES_LARGE  = 5,
    FIELD_BOAT_LIVES_HUGE   = 6
} BoatLives;
 */
static int a, b;
void FieldInit(Field *f, FieldPosition p)
{
    //iterate thru the field
    for(a = 0; a < FIELD_ROWS; a++)
    {
        for(b = 0; b < FIELD_COLS; b++)
        {
            //set all values to p (FIELD_POSITION_EMPTY/UNKNOWN/CURSOR)
            f->field[a][b] = p;
        }
    }
    //set all lives to default lives
    f->hugeBoatLives = FIELD_BOAT_LIVES_HUGE;
    f->largeBoatLives = FIELD_BOAT_LIVES_LARGE;
    f->mediumBoatLives = FIELD_BOAT_LIVES_MEDIUM;
    f->smallBoatLives = FIELD_BOAT_LIVES_SMALL;
}

/**
 * Retrieves the value at the specified field position.
 * @param f The Field being referenced
 * @param row The row-component of the location to retrieve
 * @param col The column-component of the location to retrieve
 * @return
 */
FieldPosition FieldAt(const Field *f, uint8_t row, uint8_t col)
{
    //return value at field position [row][col]
    return f->field[row][col];
}

/**
 * This function provides an interface for setting individual locations within a Field struct. This
 * is useful when FieldAddBoat() doesn't do exactly what you need. For example, if you'd like to use
 * FIELD_POSITION_CURSOR, this is the function to use.
 * 
 * @param f The Field to modify.
 * @param row The row-component of the location to modify
 * @param col The column-component of the location to modify
 * @param p The new value of the field location
 * @return The old value at that field location
 */
FieldPosition FieldSetLocation(Field *f, uint8_t row, uint8_t col, FieldPosition p)
{
    //set field[row][col] to new value (p)
    f->field[row][col] = p;
    FieldPosition newLoc = p;
    //return new FieldPosition (p)
    return newLoc;
}

/**
 * FieldAddBoat() places a single ship on the player's field based on arguments 2-5. Arguments 2, 3
 * represent the x, y coordinates of the pivot point of the ship.  Argument 4 represents the
 * direction of the ship, and argument 5 is the length of the ship being placed. All spaces that
 * the boat would occupy are checked to be clear before the field is modified so that if the boat
 * can fit in the desired position, the field is modified as SUCCESS is returned. Otherwise the
 * field is unmodified and STANDARD_ERROR is returned. There is no hard-coded limit to how many
 * times a boat can be added to a field within this function.
 *
 * So this is valid test code:
 * {
 *   Field myField;
 *   FieldInit(&myField,FIELD_POSITION_EMPTY);
 *   FieldAddBoat(&myField, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_SMALL);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_MEDIUM);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_HUGE);
 *   FieldAddBoat(&myField, 0, 6, FIELD_BOAT_DIRECTION_SOUTH, FIELD_BOAT_SMALL);
 * }
 *
 * should result in a field like:
 *  _ _ _ _ _ _ _ _
 * [ 3 3 3       3 ]
 * [ 4 4 4 4     3 ]
 * [             3 ]
 *  . . . . . . . .
 *
 * @param f The field to grab data from.
 * @param row The row that the boat will start from, valid range is from 0 and to FIELD_ROWS - 1.
 * @param col The column that the boat will start from, valid range is from 0 and to FIELD_COLS - 1.
 * @param dir The direction that the boat will face once places, from the BoatDirection enum.
 * @param boatType The type of boat to place. Relies on the FIELD_POSITION_*_BOAT values from the
 * FieldPosition enum.
 * @return TRUE for success, FALSE for failure
 */
uint8_t FieldAddBoat(Field *f, uint8_t row, uint8_t col, BoatDirection dir, BoatType type)
{
    //make sure row and col are within bounds
    int i = 0;
    if(row < 0 || row > (FIELD_ROWS - 1))
    {
        if(col < 0 || col > (FIELD_ROWS -1))
        {
            //if out of bounds, return false
            return false;
        }
    }      
    //if in bounds:
    else
    {
        switch(type)    //small, medium, large, huge
        {
// SMALL BOAT            
            case(FIELD_BOAT_SMALL):     //length of 3
                switch(dir)     //north, south, east, west
                {
                    case(FIELD_BOAT_DIRECTION_NORTH):
                        //check if field pos for boat is empty && if the field is able to fit the length of the boat
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 2][col] == FIELD_POSITION_EMPTY &&
                            (row > 1))
                        {
                            //iterate thru and set specified rows & cols to boat
                            for(i = 0; i < 3; i++)
                            {
                                f->field[row - i][col] = FIELD_POSITION_SMALL_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_EAST):
                        //check if field pos for boat is empty && if the field is able to fit the length of the boat
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 2] == FIELD_POSITION_EMPTY &&
                            ((col) < 8))
                        {
                            //iterate thru and set specified rows & cols to boat
                            for(i = 0; i < 3; i++)
                            {
                                f->field[row][col + i] = FIELD_POSITION_SMALL_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_SOUTH):
                        //check if field pos for boat is empty && if the field is able to fit the length of the boat
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 2] == FIELD_POSITION_EMPTY &&
                            (row < 4))
                        {
                            //iterate thru and set specified rows & cols to boat
                            for(i = 0; i < 3; i++)
                            {
                                f->field[row + i][col] = FIELD_POSITION_SMALL_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_WEST):
                        //check if field pos for boat is empty && if the field is able to fit the length of the boat
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 2] == FIELD_POSITION_EMPTY &&
                            (col > 1))
                        {
                            //iterate thru and set specified rows & cols to boat
                            for(i = 0; i < 3; i++)
                            {
                                f->field[row][col - i] = FIELD_POSITION_SMALL_BOAT;
                            }
                            return true;
                        }
                        break;
                }
            break;
// END: SMALL BOAT            
// MEDIUM BOAT            
            case(FIELD_BOAT_MEDIUM):    //length of 4
                switch(dir)
                {
                    case(FIELD_BOAT_DIRECTION_NORTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 3][col] == FIELD_POSITION_EMPTY &&
                            (row > 2))
                        {
                            for(i = 0; i < 4; i++)
                            {
                                f->field[row - i][col] = FIELD_POSITION_MEDIUM_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_EAST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 3] == FIELD_POSITION_EMPTY &&
                            (col < 7))
                        {
                            for(i = 0; i < 4; i++)
                            {
                                f->field[row][col + i] = FIELD_POSITION_MEDIUM_BOAT;
                            }
                            return true;
                        }
                        break;    
                    case(FIELD_BOAT_DIRECTION_SOUTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 3][col] == FIELD_POSITION_EMPTY &&
                            (row < 3))
                        {
                            for(i = 0; i < 4; i++)
                            {
                                f->field[row + i][col] = FIELD_POSITION_MEDIUM_BOAT;
                            }
                            return true;
                        }
                        break;      
                    case(FIELD_BOAT_DIRECTION_WEST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 3] == FIELD_POSITION_EMPTY &&
                            (col > 2))
                        {
                            for(i = 0; i < 4; i++)
                            {
                                f->field[row][col - i] = FIELD_POSITION_MEDIUM_BOAT;
                            }
                            return true;
                        }
                        break;    
                }
                break;
// END: MEDIUM BOAT         
// LARGE BOAT
            case(FIELD_BOAT_LARGE):     //length of 5
                switch(dir)
                {
                    case(FIELD_BOAT_DIRECTION_NORTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 3][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 4][col] == FIELD_POSITION_EMPTY &&
                            (row > 3))
                        {
                            for(i = 0; i < 5; i++)
                            {
                                f->field[row - i][col] = FIELD_POSITION_LARGE_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_EAST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 3] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 4] == FIELD_POSITION_EMPTY &&    
                            (col < 6))
                        {
                            for(i = 0; i < 5; i++)
                            {
                                f->field[row][col + i] = FIELD_POSITION_LARGE_BOAT;
                            }
                            return true;
                        }
                        break;    
                    case(FIELD_BOAT_DIRECTION_SOUTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 3][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 4][col] == FIELD_POSITION_EMPTY &&
                            (row < 2))
                        {
                            for(i = 0; i < 5; i++)
                            {
                                f->field[row + i][col] = FIELD_POSITION_LARGE_BOAT;
                            }
                            return true;
                        }
                        break;      
                    case(FIELD_BOAT_DIRECTION_WEST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 3] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 4] == FIELD_POSITION_EMPTY &&
                            (col > 3))
                        {
                            for(i = 0; i < 5; i++)
                            {
                                f->field[row][col - i] = FIELD_POSITION_LARGE_BOAT;
                            }
                            return true;
                        }
                        break;    
                }
                break;     
// END: LARGE BOAT
// HUGE BOAT                
            case(FIELD_BOAT_HUGE):      //length of 6
                switch(dir)
                {
                    case(FIELD_BOAT_DIRECTION_NORTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 3][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 4][col] == FIELD_POSITION_EMPTY &&
                            f->field[row - 5][col] == FIELD_POSITION_EMPTY &&    
                            (row == 5))
                        {
                            for(i = 0; i < 6; i++)
                            {
                                f->field[row - i][col] = FIELD_POSITION_HUGE_BOAT;
                            }
                            return true;
                        }
                        break;
                    case(FIELD_BOAT_DIRECTION_EAST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 3] == FIELD_POSITION_EMPTY &&
                            f->field[row][col + 4] == FIELD_POSITION_EMPTY && 
                            f->field[row][col + 5] == FIELD_POSITION_EMPTY &&    
                            (col < 5))
                        {
                            for(i = 0; i < 6; i++)
                            {
                                f->field[row][col + i] = FIELD_POSITION_HUGE_BOAT;
                            }
                            return true;
                        }
                        break;    
                    case(FIELD_BOAT_DIRECTION_SOUTH):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 1][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 2][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 3][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 4][col] == FIELD_POSITION_EMPTY &&
                            f->field[row + 5][col] == FIELD_POSITION_EMPTY &&    
                            (row == 1))
                        {
                            for(i = 0; i < 6; i++)
                            {
                                f->field[row + i][col] = FIELD_POSITION_HUGE_BOAT;
                            }
                            return true;
                        }
                        break;      
                    case(FIELD_BOAT_DIRECTION_WEST):
                        if(f->field[row][col] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 1] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 2] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 3] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 4] == FIELD_POSITION_EMPTY &&
                            f->field[row][col - 5] == FIELD_POSITION_EMPTY &&    
                            (col > 4))
                        {
                            for(i = 0; i < 6; i++)
                            {
                                f->field[row][col - i] = FIELD_POSITION_HUGE_BOAT;
                            }
                            return true;
                        }
                        break;    
                }
                break;  
//END: HUGE BOAT                
        }
    }  
    return false;
}

/**
 * This function registers an attack at the gData coordinates on the provided field. This means that
 * 'f' is updated with a FIELD_POSITION_HIT or FIELD_POSITION_MISS depending on what was at the
 * coordinates indicated in 'gData'. 'gData' is also updated with the proper HitStatus value
 * depending on what happened AND the value of that field position BEFORE it was attacked. Finally
 * this function also reduces the lives for any boat that was hit from this attack.
 * @param f The field to check against and update.
 * @param gData The coordinates that were guessed. The HIT result is stored in gData->hit as an
 *               output.
 * @return The data that was stored at the field position indicated by gData before this attack.
 */
FieldPosition FieldRegisterEnemyAttack(Field *f, GuessData *gData)
{
    FieldPosition guess = f->field[gData->row][gData->col];\
    
    //if guess is a HIT_HIT:
    //HUGE BOAT
    if(f->field[gData->row][gData->col] == FIELD_POSITION_HUGE_BOAT)    
    {
        //subtract a life
        f->hugeBoatLives = f->hugeBoatLives - 1; 
        //record a HIT
        gData->hit = HIT_HIT;
        //change field data to display a HIT
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
    }
    //LARGE BOAT
    else if(f->field[gData->row][gData->col] == FIELD_POSITION_LARGE_BOAT)
    {
        f->largeBoatLives = f->largeBoatLives - 1;
        gData->hit = HIT_HIT;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
    }
    //MEDIUM BOAT
    else if(f->field[gData->row][gData->col] == FIELD_POSITION_MEDIUM_BOAT)
    {
        f->mediumBoatLives = f->mediumBoatLives - 1;
        gData->hit = HIT_HIT;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
    }
    //SMALL BOAT
    else if(f->field[gData->row][gData->col] == FIELD_POSITION_SMALL_BOAT)
    {
        f->smallBoatLives = f->smallBoatLives - 1;
        gData->hit = HIT_HIT;
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
    } 
    //if guess is a HIT_MISS
    else
    {
        //record data as a MISS
        gData->hit = HIT_MISS;
        //change display to show a MISS
        f->field[gData->row][gData->col] = FIELD_POSITION_MISS;
    }
    return guess;
}

/**
 * This function updates the FieldState representing the opponent's game board with whether the
 * guess indicated within gData was a hit or not. If it was a hit, then the field is updated with a
 * FIELD_POSITION_HIT at that position. If it was a miss, display a FIELD_POSITION_EMPTY instead, as
 * it is now known that there was no boat there. The FieldState struct also contains data on how
 * many lives each ship has. Each hit only reports if it was a hit on any boat or if a specific boat
 * was sunk, this function also clears a boats lives if it detects that the hit was a
 * HIT_SUNK_*_BOAT.
 * @param f The field to grab data from.
 * @param gData The coordinates that were guessed along with their HitStatus.
 * @return The previous value of that coordinate position in the field before the hit/miss was
 * registered.
 */
FieldPosition FieldUpdateKnowledge(Field *f, const GuessData *gData)
{
    FieldPosition guess = f->field[gData->row][gData->col];
    
    //if guess is a HIT_HIT
    if(gData->hit == HIT_HIT)
    {
        //display HIT
        f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
        //if huge boat is out of lives
        if(f->hugeBoatLives == 0)
        {
            //change data to HIT_SUNK_*_BOAT
            f->hugeBoatLives = HIT_SUNK_HUGE_BOAT;
        }
        else if(f->largeBoatLives == 0)
        {
            f->largeBoatLives = HIT_SUNK_LARGE_BOAT;
        }     
        else if(f->mediumBoatLives == 0)
        {
            f->mediumBoatLives = HIT_SUNK_MEDIUM_BOAT;
        }    
        else if(f->smallBoatLives == 0)
        {
            f->smallBoatLives = HIT_SUNK_SMALL_BOAT;
        }            
    }
    else
    {
        //if not a sunk, FIELD_POSITION_EMPTY
        f->field[gData->row][gData->col] = FIELD_POSITION_EMPTY;
    }
    return guess;
}

/**
 * This function returns the alive states of all 4 boats as a 4-bit bitfield (stored as a uint8).
 * The boats are ordered from smallest to largest starting at the least-significant bit. So that:
 * 0b00001010 indicates that the small boat and large boat are sunk, while the medium and huge boat
 * are still alive. See the BoatStatus enum for the bit arrangement.
 * @param f The field to grab data from.
 * @return A 4-bit value with each bit corresponding to whether each ship is alive or not.
 */
uint8_t FieldGetBoatStates(const Field *f)
{
    uint8_t states = 0;
    
    //if boats still have lives:
    if(f->smallBoatLives != 0)
    {
        states |= FIELD_BOAT_STATUS_SMALL;
    }
    if(f->mediumBoatLives != 0)
    {
        states |= FIELD_BOAT_STATUS_MEDIUM;
    }
    if(f->largeBoatLives != 0)
    {
        states |= FIELD_BOAT_STATUS_LARGE;
    }
    if(f->hugeBoatLives != 0)
    {
        states |= FIELD_BOAT_STATUS_HUGE;
    }
    return states;
}