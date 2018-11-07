// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Protocol.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****


// **** Declare any function prototypes here ****
NegotiationData *nData;
GuessData *gData;

ProtocolParserStatus ppstest1;

int main()
{
    BOARD_Init();
/******************************************************************************
 * Your code goes in between this comment and the following one with asterisks.
 * 
 * typedef enum {
 *  PROTOCOL_PARSING_FAILURE = -1, // Parsing failed for some reason. Could signify and unknown
 *                                 // message was received or the checksum was incorrect.
 *  PROTOCOL_WAITING,              // Parsing is waiting for the starting '$' of a new message
 *  PROTOCOL_PARSING_GOOD,         // A success value that indicates no message received yet.
 *  
 *  PROTOCOL_PARSED_COO_MESSAGE,   // Coordinate message. This is used for exchanging guesses.
 *  PROTOCOL_PARSED_HIT_MESSAGE,   // Hit message. Indicates a response to a Coordinate message.
 *  PROTOCOL_PARSED_CHA_MESSAGE,   // Challenge message. Used in the first step of negotiating the
 *                                 // turn order.
 *  PROTOCOL_PARSED_DET_MESSAGE    // Determine message. Used in the second and final step of
 *                                 // negotiating the turn order.
 * } ProtocolParserStatus;
 *****************************************************************************/
    int x;
    x = 0;
    
    //TEST 1 INPUT STRING
    //char test1[PROTOCOL_MAX_PAYLOAD_LEN + 1] = {"$CHA,37348,117*46"};
    //char test1[PROTOCOL_MAX_PAYLOAD_LEN + 1] = {"$COO,3,5*45"};
    //char test1[PROTOCOL_MAX_PAYLOAD_LEN + 1] = {"$DET,32990,21382*5e"};
    //char test1[PROTOCOL_MAX_PAYLOAD_LEN + 1] = {"$HIT,1,7,1*4e"};
    char test1[PROTOCOL_MAX_PAYLOAD_LEN + 1] = {"$COO,3,5*45"};
    
    while (1){
    ppstest1 = ProtocolDecode(*test1, nData, gData);
    if(ppstest1 == PROTOCOL_PARSING_FAILURE){
        printf("FAILURE ");
    } else if (ppstest1 == PROTOCOL_WAITING){
        printf("WAITING ");
    } else if (ppstest1 == PROTOCOL_PARSING_GOOD){
        printf("GOOD ");
    } else if (ppstest1 == PROTOCOL_PARSED_COO_MESSAGE){
        printf("COO ");
    } else if (ppstest1 == PROTOCOL_PARSED_HIT_MESSAGE){
        printf("HIT ");
    } else if (ppstest1 == PROTOCOL_PARSED_CHA_MESSAGE){
        printf("CHA ");
    } else if (ppstest1 == PROTOCOL_PARSED_DET_MESSAGE){
        printf("DET ");
    } 
    x++;
    *test1 = test1[x];
    }
    
    
    
    

/******************************************************************************
 * Your code goes in between this comment and the preceeding one with asterisks
 *****************************************************************************/
    while (1);
}