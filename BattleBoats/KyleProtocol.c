/*
 * File:   Protocol.c
 * Author: kyleko
 *
 * Created on June 2, 2018, 8:46 PM
 * 
 * Partner: Aaron Lethers
 */

//CMPE13 Support Library
#include "BOARD.h"

// Standard libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Microchip libraries
#include <xc.h>

//Protocol Header File
#include "Protocol.h"
#include "Agent.h"

// **** Set any macros or preprocessor directives here ****
#define TRUE 1
#define FALSE 0
#define HashInit() \
        tempHash = 0; \
        tempHash ^= (data->guess >> 8); \
        tempHash ^= data->guess; \
        tempHash ^= (data->encryptionKey >> 8); \
        tempHash ^= data->encryptionKey; \
        tempHash &= 0x000000FF;
#define Refresh() \
        ClearString(ProData.sentence); \
        ProData.msgIndex = 0; 

// **** Declare any function prototypes here ****
static uint8_t checkSum(char *proto); // encodes checksum
static uint8_t AsciiToHex(char ascii); // ascii to hex converter, returns -1 if invalid character
static void ClearString(char *string);
static uint8_t CheckChecksum(char *string);

// TRANSITIONS FOR STATE MACHINE IN ProtocolDecode()
typedef enum{
    WAITING,
    RECORDING,
    FIRST_CHECKSUM_HALF,
    SECOND_CHECKSUM_HALF,
    NEWLINE
}DecodeState;

// Struct for ProtocolDecode,
typedef struct PD {
    DecodeState Dstate;
    char sentence[PROTOCOL_MAX_PAYLOAD_LEN];
    uint8_t msgIndex;
    uint8_t checks;
}PD;

// **** Define any module-level, global, or external variables here ****
static PD ProData = {WAITING,"\0", 0, 0}; // Initialized PD Struct, NAME: ProData
static uint8_t hex, checkStatus;
static int caseNum;



/**
 * Encodes the coordinate data for a guess into the string `message`. This string must be big
 * enough to contain all of the necessary data. The format is specified in PAYLOAD_TEMPLATE_COO,
 * which is then wrapped within the message as defined by MESSAGE_TEMPLATE. The final length of this
 * message is then returned. There is no failure mode for this function as there is no checking
 * for NULL pointers.
 * @param message The character array used for storing the output. Must be long enough to store the
 *                entire string, see PROTOCOL_MAX_MESSAGE_LEN.
 * @param data The data struct that holds the data to be encoded into `message`.
 * @return The length of the string stored into `message`.
 * 
 * 
 * From header:
 *   Defined below are the various messages used by the protocol. Each follows the NMEA0183 syntax for
 *   messages, with the exclusion of the Talked ID portion.
 *   #define PAYLOAD_TEMPLATE_HIT "HIT,%u,%u,%u" // Hit message: row, col, hit (see HitStatus)
 *   #define PAYLOAD_TEMPLATE_COO "COO,%u,%u"    // Coordinate message: row, col
 *   #define PAYLOAD_TEMPLATE_CHA "CHA,%u,%u"    // Challenge message: encryptedGuess, hash
 *   #define PAYLOAD_TEMPLATE_DET "DET,%u,%u"    // Determine message: guess, encryptionKey
 * 
 *   #define MESSAGE_TEMPLATE "$%s*%02x\n"
 * 
 * 
 */

/**/
int ProtocolEncodeCooMessage(char *message, const GuessData *data){
    char coord[PROTOCOL_MAX_PAYLOAD_LEN] = {};
    sprintf(coord, PAYLOAD_TEMPLATE_COO, data->row, data->col);
    
    printf("Coordinate Info: %s\n", coord);
    
    return sprintf(message, MESSAGE_TEMPLATE, coord, checkSum(coord));
    //return strlen(message);
}

int ProtocolEncodeHitMessage(char *message, const GuessData *data){
    char hit[PROTOCOL_MAX_PAYLOAD_LEN] = {};
    sprintf(hit, PAYLOAD_TEMPLATE_HIT, data->row, data->col, data->hit);
    
    printf("Hit Info: %s\n", hit);
    
    return sprintf(message, MESSAGE_TEMPLATE, hit, checkSum(hit));
    //return strlen(message);
}

int ProtocolEncodeChaMessage(char *message, const NegotiationData *data){
    char challenge[PROTOCOL_MAX_PAYLOAD_LEN] = {};
    sprintf(challenge, PAYLOAD_TEMPLATE_CHA, data->encryptedGuess, data->hash);
    
    printf("Challenge Info: %s\n", challenge);
    
    return sprintf(message, MESSAGE_TEMPLATE, challenge, checkSum(challenge));
    //return strlen(message);
    
}

int ProtocolEncodeDetMessage(char *message, const NegotiationData *data){
    char determine[PROTOCOL_MAX_PAYLOAD_LEN] = {};
    sprintf(determine, PAYLOAD_TEMPLATE_DET, data->guess, data->encryptionKey);
    
    printf("Determine Info: %s\n", determine);
    
    return sprintf(message, MESSAGE_TEMPLATE, determine, checkSum(determine));
    //return strlen(message);
}

/**
 * This function decodes a message into either the NegotiationData or GuessData structs depending
 * on what the type of message is. This function receives the message one byte at a time, where the
 * messages are in the format defined by MESSAGE_TEMPLATE, with payloads of the format defined by
 * the PAYLOAD_TEMPLATE_* macros. It returns the type of message that was decoded and also places
 * the decoded data into either the `nData` or `gData` structs depending on what the message held.
 * The onus is on the calling function to make sure the appropriate structs are available (blame the
 * lack of function overloading in C for this ugliness).
 *
 * PROTOCOL_PARSING_FAILURE is returned if there was an error of any kind (though this excludes
 * checking for NULL pointers), while
 * 
 * @param in The next character in the NMEA0183 message to be decoded.
 * @param nData A struct used for storing data if a message is decoded that stores NegotiationData.
 * @param gData A struct used for storing data if a message is decoded that stores GuessData.
 * @return A value from the UnpackageDataEnum enum.
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
 *
 * typedef enum{
 *  WAITING,
 *  RECORDING,
 *  FIRST_CHECKSUM_HALF,
 *  SECOND_CHECKSUM_HALF,
 *  NEWLINE
 * } DecodeState; 
 * 
 * 
 */


ProtocolParserStatus ProtocolDecode(char in, NegotiationData *nData, GuessData *gData){
    // Initialized above: 
    // static PD ProData = {WAITING,"\0", 0, 0}; // Initialized PD Struct, NAME: ProData
    // Implement State Machine in lab manual
    switch(ProData.Dstate){
        case WAITING:
            if(in == '$'){
                Refresh();
                ProData.Dstate = RECORDING;
                printf("Waiting, good\n"); // Unit Testing
                return PROTOCOL_PARSING_GOOD;
            }
            else if (in != '$'){
                ProData.Dstate = WAITING; // TRANSITION
                return PROTOCOL_WAITING;
            }
            break; 
            
        case RECORDING:
            if(in == '*'){
                ProData.Dstate = FIRST_CHECKSUM_HALF; // TRANSITION
                printf("Recording, good\n");
                return PROTOCOL_PARSING_GOOD;
            } else if (ProData.msgIndex >= PROTOCOL_MAX_PAYLOAD_LEN) { // if length too long
                printf("FAIL 1, STRING OUT OF BOUNDS\n");
                ProData.Dstate = WAITING;
                return PROTOCOL_PARSING_FAILURE;
            } else if (in != '*'){
                ProData.sentence[ProData.msgIndex] = in;
                ProData.msgIndex++;
                ProData.Dstate = RECORDING;
                return PROTOCOL_PARSING_GOOD;
            }
            break;
                    
        case FIRST_CHECKSUM_HALF:
            hex = AsciiToHex(in); // returns -1 if invalid
            
            if(hex == -1){ // if invalid hex character is received
                printf("FAIL 2\n");
                ProData.Dstate = WAITING;
                return PROTOCOL_PARSING_FAILURE;
            } else { // valid hex character received (a-f, A-F, 0-9)
                ProData.Dstate = SECOND_CHECKSUM_HALF; // TRANSITION
                ProData.checks = hex; 
                ProData.checks <<= 4;
                printf("FirstCheckSum, good\n");
                return PROTOCOL_PARSING_GOOD;
            }
            break;
            
        case SECOND_CHECKSUM_HALF:
            hex = AsciiToHex(in);
            ProData.checks |= hex;
            checkStatus = CheckChecksum(ProData.sentence); // returns TRUE/FALSE if checksums are equal
            
            if((hex == -1) || (checkStatus != ProData.checks)){
                printf("FAIL 3\n");
                ProData.Dstate = WAITING;
                return PROTOCOL_PARSING_FAILURE;
            } 
            else if((hex != -1) && (checkStatus == ProData.checks)){
                ProData.Dstate = NEWLINE;
                ProData.sentence[ProData.msgIndex] = '\0';
                printf("SecondChecksum, good\n");
                return PROTOCOL_PARSING_GOOD;
            }
            
            break;
            
        case NEWLINE:
            // Translate STRING, and return PARSED MESSAGE
            // ex. $COO,3,5*45
            // Tokenize before , and *
            
            
            if (in == '\n') {
                printf("IN!\n");
                // CaseNum changes based off of Input String. 1 = HIT, 2 = COO, 3 = CHA, 4 = DET
                if (strncmp(ProData.sentence, PAYLOAD_TEMPLATE_HIT, 3) == 0) {
                    caseNum = 1;
                } else if (strncmp(ProData.sentence, PAYLOAD_TEMPLATE_COO, 3) == 0) {
                    caseNum = 2; 
                } else if (strncmp(ProData.sentence, PAYLOAD_TEMPLATE_CHA, 3) == 0) {
                    caseNum = 3;       
                } else if (strncmp(ProData.sentence, PAYLOAD_TEMPLATE_DET, 3) == 0) {
                    caseNum = 4; 
                } else {
                    caseNum = 0;
                }
                
                printf("%d ", caseNum);
            
                // HIT
                if(caseNum == 1){
                    if(sscanf(ProData.sentence, PAYLOAD_TEMPLATE_HIT, 
                        &gData->row, &gData->col, &gData->hit) == 3){
                            ProData.Dstate = WAITING;                          
                            return PROTOCOL_PARSED_HIT_MESSAGE;
                    }
                    
                }
                
                // COO
                else if (caseNum == 2){
                    if(sscanf(ProData.sentence, PAYLOAD_TEMPLATE_COO,
                        &gData->row, &gData->col) == 2){
                        ProData.Dstate = WAITING;                      
                        return PROTOCOL_PARSED_COO_MESSAGE;
                    }
                }
                    
                // CHA
                else if (caseNum == 3){
                    if (sscanf(ProData.sentence, PAYLOAD_TEMPLATE_CHA,
                        &nData->encryptedGuess, &nData->hash) == 2) {
                        ProData.Dstate = WAITING;
                        return PROTOCOL_PARSED_CHA_MESSAGE;
                    }
                }
                // DET
                else if (caseNum == 4){
                    if (sscanf(ProData.sentence, PAYLOAD_TEMPLATE_DET,
                        &nData->guess, &nData->encryptionKey) == 2) {
                        ProData.Dstate = WAITING;
                        return PROTOCOL_PARSED_DET_MESSAGE;
                    }
                }
                //0
                else if (caseNum == 0){
                    printf("FAIL 4\n");
                    printf("CASE 0\n");
                    ProData.Dstate = WAITING;
                    return PROTOCOL_PARSING_FAILURE;
                }
            
            } else {
                printf("NOT IN\n");
                printf("FAIL 4\n");
            // Resets State and returns failure if in != '\n' or if MSGID is not valid
            ProData.Dstate = WAITING;
            return PROTOCOL_PARSING_FAILURE;
            break;
            }
            break;
            
        default:
            return PROTOCOL_PARSING_FAILURE;
            break;
    } 
    return PROTOCOL_PARSING_FAILURE;
}

/**
 * This function generates all of the data necessary for the negotiation process used to determine
 * the player that goes first. It relies on the pseudo-random functionality built into the standard
 * library. The output is stored in the passed NegotiationData struct. The negotiation data is
 * generated by creating two random 16-bit numbers, one for the actual guess and another for an
 * encryptionKey used for encrypting the data. The 'encryptedGuess' is generated with an
 * XOR(guess, encryptionKey). The hash is simply an 8-bit value that is the XOR() of all of the
 * bytes making up both the guess and the encryptionKey. There is no checking for NULL pointers
 * within this function.
 * @param data The struct used for both input and output of negotiation data.
 */ 
void ProtocolGenerateNegotiationData(NegotiationData *data){
    uint8_t tempHash;
    
    data->guess = (rand() & 0xFFFF);
    data->encryptionKey = (rand() & 0xFFFF);
    data->encryptedGuess = (data->guess ^ data->encryptionKey);
    
    // XOR with both guess and encryptionKey
    HashInit();
    
    /* defined for macro usage
    tempHash = 0;
    tempHash ^= (data->guess >> 8);
    tempHash ^= data->guess;
    tempHash ^= (data->encryptionKey >> 8);
    tempHash ^= data->encryptionKey;
    tempHash &= 0x000000FF; // last 2 bytes only
    */
    
    data->hash = tempHash;
}
/**
 * Validates that the negotiation data within 'data' is correct according to the algorithm given in
 * GenerateNegotitateData(). Used for verifying another agent's supplied negotiation data. There is
 * no checking for NULL pointers within this function. Returns TRUE if the NegotiationData struct
 * is valid or FALSE on failure.
 * @param data A filled NegotiationData struct that will be validated.
 * @return TRUE if the NegotiationData struct is consistent and FALSE otherwise.
 */
uint8_t ProtocolValidateNegotiationData(const NegotiationData *data){
    printf("Validate\n");
    uint8_t tempHash;
    HashInit(); // XOR reverses encryption too.
    if((data->encryptionKey ^ data->encryptedGuess) != data->guess){
        return FALSE;
    } else if (tempHash != data->hash){
        return FALSE;
    } else {
        return TRUE;
    }
}

/**
 * This function returns a TurnOrder enum type representing which agent has won precedence for going
 * first. The value returned relates to the agent whose data is in the 'myData' variable. The turn
 * ordering algorithm relies on the XOR() of the 'encryptionKey' used by both agents. The least-
 * significant bit of XOR(myData.encryptionKey, oppData.encryptionKey) is checked so that if it's a
 * 1 the player with the largest 'guess' goes first otherwise if it's a 0, the agent with the
 * smallest 'guess' goes first. The return value of TURN_ORDER_START indicates that 'myData' won,
 * TURN_ORDER_DEFER indicates that 'oppData' won, otherwise a tie is indicated with TURN_ORDER_TIE.
 * There is no checking for NULL pointers within this function.
 * @param myData The negotiation data representing the current agent.
 * @param oppData The negotiation data representing the opposing agent.
 * @return A value from the TurnOrdering enum representing which agent should go first.
 * 
 * typedef enum {
 *   TURN_ORDER_TIE = -1,
 *   TURN_ORDER_DEFER = 0,
 *   TURN_ORDER_START = 1
 *   } TurnOrder;
 */
TurnOrder ProtocolGetTurnOrder(const NegotiationData *myData, const NegotiationData *oppData){
    printf("TurnOrder \n");
    uint8_t turn;
    turn = 0x01 & (myData->encryptionKey ^ oppData->encryptionKey); // least sig bit, XOR'd
    
    if(turn == 0x01){
        if (myData->guess > oppData->guess){
            return TURN_ORDER_START;
        } else {
            return TURN_ORDER_DEFER;
        }
    } else {
        if (myData->guess < oppData->guess){
            return TURN_ORDER_START;
        } else {
            return TURN_ORDER_DEFER;
        }
    }
    return TURN_ORDER_TIE;
}



// HELPER FUNCTIONS
static uint8_t checkSum(char *proto){
    uint8_t checkSum = 0; // declare,initiate in function to return
    int i;
    
    for(i = 0; i <= strlen(proto); i++){
        checkSum ^= proto[i];
    }

    return checkSum; // will return checkSum
}

static uint8_t AsciiToHex(char ascii){
    printf("AtoH\n");
    uint8_t hex;
    
    if((ascii >= 0x30) && (ascii <= 0x39)){ // 0x30 = 0, 0x39 = 9
        hex = (uint8_t)ascii - 48;
    } else if ((ascii >= 0x41) && (ascii <= 0x46)){ // 0x41 = 'A', 0x46 = 'F'
        hex = (uint8_t)ascii - 55; // - 65 +
    } else if ((ascii >= 0x61) && (ascii <= 0x66)){ // 0x61 = 'a', 0x66 = 'f'
        hex = (uint8_t)ascii - 87;
    } else {
        hex = -1;
        printf("Invalid String\n");
    }
    
    return hex;
    
}

static uint8_t CheckChecksum(char *string)
{
    uint8_t csum = 0;
    while (*string != '\0') {
        csum ^= *string;
        string++;
    }
    return csum;
}
static void ClearString(char *string){
    while(*string != '\0'){
        *string = '\0';
        string++;
    }
}
