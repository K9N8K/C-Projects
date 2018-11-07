/*
 * File:   ArtificialAgent.c
 * Author: kyleko
 *
 * Created on June 5, 2018, 3:52 PM
 */

// Standard Libraries
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Agent Header File
#include "Agent.h"

// User Defined Libraries
#include "Field.h"
#include "Oled.h"
#include "Protocol.h"
#include "OledDriver.h"
#include "FieldOled.h"
#include "Leds.h"
#include "Buttons.h"

#define TRUE 1
#define FALSE 0

static Field Self, Opp;
static int row, col, ranDir;
static bool smallStatus, mediumStatus, largeStatus, hugeStatus;
static BoatDirection direction;


/**
 * The Init() function for an Agent sets up everything necessary for an agent before the game
 * starts. This can include things like initialization of the field, placement of the boats,
 * etc. The agent can assume that stdlib's rand() function has been seeded properly in order to
 * use it safely within.
 */
void AgentInit(void){
    //INIT LEDs
    LEDS_INIT();
    
    //Init Field, empty for both players
    FieldInit(&Self, FIELD_POSITION_EMPTY);
    FieldInit(&Opp, FIELD_POSITION_UNKNOWN);
    
    smallStatus = false;
    mediumStatus = false;
    largeStatus = false;
    hugeStatus = false;
    
    // while all boats have not been placed yet
    while(smallStatus != true || mediumStatus != true || largeStatus != true || hugeStatus != true){
    
        // assign new random location  
        row = rand() % FIELD_ROWS;
        col = rand() % FIELD_COLS;
        ranDir = rand() % 4; // 4 directions, 1 = N, 2 = E, 3 = S, 4 = W
    
        // assign new random direction
        switch(ranDir){
            case 1: direction = FIELD_BOAT_DIRECTION_NORTH; break;
            case 2: direction = FIELD_BOAT_DIRECTION_EAST; break;
            case 3: direction = FIELD_BOAT_DIRECTION_SOUTH; break;
            case 4: direction = FIELD_BOAT_DIRECTION_WEST; break;
        }
    
        if(smallStatus != true){
            smallStatus = FieldAddBoat(&Self, row, col, direction, FIELD_BOAT_SMALL);
        } else if (mediumStatus != true){
            mediumStatus = FieldAddBoat(&Self, row, col, direction,FIELD_BOAT_MEDIUM);
        } else if (largeStatus != true){
            largeStatus = FieldAddBoat(&Self, row, col, direction, FIELD_BOAT_LARGE);
        } else if (hugeStatus != true){
            hugeStatus = FieldAddBoat(&Self, row, col, direction, FIELD_BOAT_HUGE);
        }
    
    }
    
    FieldOledDrawScreen(&Self, &Opp, FIELD_OLED_TURN_NONE);
    
}

/**
 * The Run() function for an Agent takes in a single character. It then waits until enough
 * data is read that it can decode it as a full sentence via the Protocol interface. This data
 * is processed with any output returned via 'outBuffer', which is guaranteed to be 255
 * characters in length to allow for any valid NMEA0183 messages. The return value should be
 * the number of characters stored into 'outBuffer': so a 0 is both a perfectly valid output and
 * means a successful run.
 * @param in The next character in the incoming message stream.
 * @param outBuffer A string that should be transmit to the other agent. NULL if there is no
 *                  data.
 * @return The length of the string pointed to by outBuffer (excludes \0 character).
 *
 * typedef enum {
    AGENT_STATE_GENERATE_NEG_DATA,
    AGENT_STATE_SEND_CHALLENGE_DATA,
    AGENT_STATE_DETERMINE_TURN_ORDER,
    AGENT_STATE_SEND_GUESS,
    AGENT_STATE_WAIT_FOR_HIT,
    AGENT_STATE_WAIT_FOR_GUESS,
    AGENT_STATE_INVALID,
    AGENT_STATE_LOST,
    AGENT_STATE_WON
    } Astate;
 * 
 * 
 * 
 */

static int returnValue, timeDelay;
static NegotiationData myData;
static NegotiationData oppData;
static GuessData myGuess;
static GuessData oppGuess;

static TurnOrder turn;
static FieldOledTurn turnState = FIELD_OLED_TURN_NONE;

static bool guessSuc;

static  AgentState Astate = AGENT_STATE_GENERATE_NEG_DATA;

static ProtocolParserStatus returnedMsg;
static AgentEvent ProtocolFlag;

static uint8_t MyBoats;

int AgentRun(char in, char *outBuffer){
    
    // Advice from TA: Create a flag that characterizes the returns in Protocol
    if(in != '\0'){
        
        returnedMsg = ProtocolDecode(in, &oppData, &oppGuess);
        if (returnedMsg == (PROTOCOL_PARSING_GOOD || PROTOCOL_WAITING)) { // check the various states to see what the protocol status is
            ProtocolFlag = AGENT_EVENT_NONE;
            printf("1  ");
        } else if (returnedMsg == PROTOCOL_PARSED_CHA_MESSAGE) {
            ProtocolFlag = AGENT_EVENT_RECEIVED_CHA_MESSAGE;
            printf("2  ");
        } else if (returnedMsg == PROTOCOL_PARSED_COO_MESSAGE) {
            ProtocolFlag = AGENT_EVENT_RECEIVED_COO_MESSAGE;
            printf("3  ");
        } else if (returnedMsg == PROTOCOL_PARSED_DET_MESSAGE) {
            ProtocolFlag = AGENT_EVENT_RECEIVED_DET_MESSAGE;
            printf("4  ");
        } else if (returnedMsg == PROTOCOL_PARSED_HIT_MESSAGE) {
            ProtocolFlag = AGENT_EVENT_RECEIVED_HIT_MESSAGE;
            printf("5  ");
        } else if (returnedMsg == PROTOCOL_PARSING_FAILURE){
            ProtocolFlag = AGENT_EVENT_MESSAGE_PARSING_FAILED;
            printf("6  ");
        }
        
    } 
    
    switch(Astate)
    {
        case(AGENT_STATE_GENERATE_NEG_DATA):
            printf("AGENT_STATE_GENERATE_NEG_DATA\n");
            Astate = AGENT_STATE_SEND_CHALLENGE_DATA;
            //generate negotiation data
            ProtocolGenerateNegotiationData(&myData);
            //send challenge data
            
            returnValue = ProtocolEncodeChaMessage(outBuffer, &myData);
            
            return returnValue;
            break;
            
        case(AGENT_STATE_SEND_CHALLENGE_DATA):
            printf("AGENT_STATE_SEND_CHALLENGE_DATA\n");
            
            if(ProtocolFlag == AGENT_EVENT_RECEIVED_CHA_MESSAGE){ // Challenge 
                ProtocolFlag = AGENT_EVENT_RECEIVED_DET_MESSAGE;
                Astate = AGENT_STATE_DETERMINE_TURN_ORDER;
                returnValue = ProtocolEncodeDetMessage(outBuffer, &myData);
                return returnValue;
            }
            else if (ProtocolFlag == AGENT_EVENT_MESSAGE_PARSING_FAILED){
                OledClear(OLED_COLOR_BLACK);
                OledDrawString("PARSING ERROR 1");
                OledUpdate();
                FATAL_ERROR();
            }
            else { 
                return 0;
            }
            break;
            
        case(AGENT_STATE_DETERMINE_TURN_ORDER):
            //printf("AGENT_STATE_DETERMINE_TURN_ORDER\n");
            //get DETERMINE return
            
            if(ProtocolFlag == AGENT_EVENT_RECEIVED_DET_MESSAGE){
                //printf("yoyooyo\n");
                
                if(ProtocolValidateNegotiationData(&oppData)){
                    turn = ProtocolGetTurnOrder(&myData, &oppData);
                    // SELF TURN
                    if(turn == TURN_ORDER_START){
                        Astate = AGENT_STATE_SEND_GUESS;
                        turnState = FIELD_OLED_TURN_MINE;
                        FieldOledDrawScreen(&Self, &Opp, turnState);
                        OledUpdate();
                        return 0;
                    }
                   // OPP TURN
                    else if(turn == TURN_ORDER_DEFER){
                        Astate = AGENT_STATE_WAIT_FOR_GUESS;
                        turnState = FIELD_OLED_TURN_THEIRS;
                        FieldOledDrawScreen(&Self, &Opp, turnState);
                        OledUpdate();
                        return 0;
                    }
                    // TIE
                    else {
                        Astate = AGENT_STATE_INVALID;
                        OledClear(OLED_COLOR_BLACK);
                        OledDrawString("TIED TURN");
                        OledUpdate();
                        return 0;
                    }
                   
                    
                } else {
                    Astate= AGENT_STATE_INVALID;
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString("NEGOTIATION ERROR ");
                    OledUpdate();
                    FATAL_ERROR();
                }
                
                //return 0;
                
            } else if (ProtocolFlag == AGENT_EVENT_MESSAGE_PARSING_FAILED){
                OledClear(OLED_COLOR_BLACK);
                OledDrawString("PARSING ERROR 2");
                OledUpdate();
                FATAL_ERROR();
            }
            
            if (ProtocolFlag == AGENT_EVENT_NONE){
                printf("None nigga\n");
            } else if (ProtocolFlag == AGENT_EVENT_RECEIVED_CHA_MESSAGE) {
                printf("Cha nigga\n");
            } else if (ProtocolFlag == AGENT_EVENT_RECEIVED_COO_MESSAGE) {
                printf("Coo nigga\n");
            } else if (ProtocolFlag == AGENT_EVENT_RECEIVED_DET_MESSAGE) {
                printf("Det nigga\n");
            } else if (ProtocolFlag == AGENT_EVENT_RECEIVED_HIT_MESSAGE) {
                printf("Hit nigga\n");
            } else if (ProtocolFlag == AGENT_EVENT_MESSAGE_PARSING_FAILED){
                printf("Failed nigga\n");
            }
            
            
            printf("baodsifoadsf\n");
            break;
            
            
        case(AGENT_STATE_SEND_GUESS):
            printf("AGENT_STATE_SEND_GUESS\n");
            
            // time delay
            for (timeDelay = 0; timeDelay < (BOARD_GetPBClock() / 8); timeDelay++);
            
            //generate valid coordinates
            guessSuc = false;
            while(guessSuc == false)
            {
                printf("yo");
                myGuess.row = rand() % 6;
                myGuess.col = rand() % 10;
                if(Opp.field[myGuess.row][myGuess.col] == FIELD_POSITION_UNKNOWN)
                {
                    guessSuc = true;
                }
                else
                {
                    guessSuc = false;
                }
            }
   
            returnValue = ProtocolEncodeCooMessage(outBuffer, &myGuess);
            Astate = AGENT_STATE_WAIT_FOR_HIT;
            return returnValue;
            break;
        
            
            // checkpoint
        case(AGENT_STATE_WAIT_FOR_HIT):
            printf("AGENT_STATE_WAIT_FOR_HIT\n");
 
            if(ProtocolFlag == AGENT_EVENT_RECEIVED_HIT_MESSAGE){
                FieldUpdateKnowledge(&Opp, &oppGuess);
                
                //Check if there are still boats, if not -> WIN/LOSS
                if(FieldGetBoatStates(&Opp) > 0){
                    Astate = AGENT_STATE_WAIT_FOR_GUESS;
                    turnState = FIELD_OLED_TURN_THEIRS;
                    FieldOledDrawScreen(&Self, &Opp, turnState);
                    OledUpdate();
                    return 0;
                } else {
                    Astate = AGENT_STATE_WON;
                    turnState = FIELD_OLED_TURN_NONE;
                    FieldOledDrawScreen(&Self, &Opp, turnState);
                    OledUpdate();
                    return 0;
                }
       
            }
            
            else if (ProtocolFlag == AGENT_EVENT_MESSAGE_PARSING_FAILED){
                OledClear(OLED_COLOR_BLACK);
                OledDrawString("PARSING ERROR 3");
                OledUpdate();
                FATAL_ERROR();
            }
            
            return 0;
            break;
            
        case(AGENT_STATE_WAIT_FOR_GUESS): 
            printf("AGENT_STATE_WAIT_FOR_GUESS\n");
            
            MyBoats = AgentGetStatus();
            // MyBoats will be TRUE if Self has Boats alive
            // MyBoats will be FALSE if Self has no Boats alive
            
            if((ProtocolFlag == AGENT_EVENT_RECEIVED_COO_MESSAGE) && (MyBoats == TRUE)){
                //ProtocolFlag = 0;
                Astate = AGENT_STATE_SEND_GUESS;
                turnState = FIELD_OLED_TURN_MINE;
                returnValue = ProtocolEncodeHitMessage(outBuffer, &oppGuess);
                FieldRegisterEnemyAttack(&Self, &oppGuess);
                FieldOledDrawScreen(&Self, &Opp, turnState);
                return returnValue;
                
            } else if ((ProtocolFlag == AGENT_EVENT_RECEIVED_COO_MESSAGE) && (MyBoats == FALSE)){
                //ProtocolFlag = 0;
                Astate = AGENT_STATE_LOST;
                turnState = FIELD_OLED_TURN_NONE;
                returnValue = ProtocolEncodeHitMessage(outBuffer, &oppGuess);
                FieldOledDrawScreen(&Self, &Opp, turnState);
                return returnValue;
                
            } else if (ProtocolFlag == AGENT_EVENT_MESSAGE_PARSING_FAILED){
                OledClear(OLED_COLOR_BLACK);
                OledDrawString("PARSING ERROR 4");
                OledUpdate();
                FATAL_ERROR();
            }
            else {
                return 0;
            }
            break;
            
            
        case AGENT_STATE_WON:
            return 0;
            break;
        case AGENT_STATE_LOST:
            return 0;
            break;
        case AGENT_STATE_INVALID:
            //return 0;
            break;
        default:
            return 0;
        
    }
    return 0;
}

/**
 * StateCheck() returns a 4-bit number indicating the status of that agent's ships. The smallest
 * ship, the 3-length one, is indicated by the 0th bit, the medium-length ship (4 tiles) is the
 * 1st bit, etc. until the 3rd bit is the biggest (6-tile) ship. This function is used within
 * main() to update the LEDs displaying each agents' ship status. This function is similar to
 * Field::FieldGetBoatStates().
 * @return A bitfield indicating the sunk/unsunk status of each ship under this agent's control.
 *
 * @see Field.h:FieldGetBoatStates()
 * @see Field.h:BoatStatus
 */
uint8_t AgentGetStatus(void){
    //return user field
    return FieldGetBoatStates(&Self);
}

/**
 * This function returns the same data as `AgentCheckState()`, but for the enemy agent.
 * @return A bitfield indicating the sunk/unsunk status of each ship under the enemy agent's
 *         control.
 *
 * @see Field.h:FieldGetBoatStates()
 * @see Field.h:BoatStatus
 */
uint8_t AgentGetEnemyStatus(void){
    //return enemy field
    return FieldGetBoatStates(&Opp);
}
