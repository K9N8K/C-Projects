/*
 * File:   Morse.c
 * Author: kyleko
 *
 * Created on May 28, 2018, 2:41 PM
 */


#include "Morse.h"
#include "BinaryTree.h"
#include "BOARD.h"
#include "Buttons.h"

typedef enum{
    WAITING,
    DOT,
    DASH,
    INTER_LETTER,
} state;

static const char morseTree[] = {'\0', // ROOT

                       'E', // DOT ROOT
                       // DOT -> DOT TREE
                       'I','S','H','5','4','V','\0','3','U','F','\0','\0','\0','\0','2',
                       // DOT -> DASH TREE
                       'A','R','L','\0','\0','\0','\0','\0','W','P','\0','\0','J','\0','1',
                     
                       'T', // DASH ROOT
                       //DASH -> DOT
                       'N','D','B','6','\0','X','\0','\0','K','C','\0','\0','Y','\0','\0',
                       //DASH -> DASH
                       'M','G','Z','7','\0','Q','\0','\0','O','\0','8','\0','\0','9','0'};

static int timer;
state morseState = WAITING;
uint8_t buttonEvent;
static Node *morse = NULL;
static Node *morseRoot = NULL;


/**
 * This function initializes the Morse code decoder. This is primarily the generation of the
 * Morse tree: a binary tree consisting of all of the ASCII alphanumeric characters arranged
 * according to the DOTs and DASHes that represent each character. Traversal of the tree is done by
 * taking the left-child if it is a dot and the right-child if it is a dash. If the morse tree is
 * successfully generated, SUCCESS is returned, otherwise STANDARD_ERROR is returned. This function
 * also initializes the Buttons library so that MorseCheckEvents() can work properly.
 * @return Either SUCCESS if the decoding tree was successfully created or STANDARD_ERROR if not.
 */ 
int MorseInit(void){
    
    ButtonsInit();
    morse = TreeCreate(6, morseTree);
    morseRoot = morse;
    
    if(morse != NULL){
        return SUCCESS;
    }
    else {
        return STANDARD_ERROR;
    }
    
}

/**
 * MorseDecode decodes a Morse string by iteratively being passed MORSE_CHAR_DOT or MORSE_CHAR_DASH.
 * Each call that passes a DOT or DASH returns a SUCCESS if the string could still compose a
 * Morse-encoded character. Passing in the special MORSE_CHAR_END_OF_CHAR constant will terminate
 * decoding and return the decoded character. During that call to MorseDecode() the return value
 * will be the character that was decoded or STANDARD_ERROR if it couldn't be decoded. Another
 * special value exists, MORSE_CHAR_DECODE_RESET, which will clear the stored state. When a
 * MORSE_CHAR_DECODE_RESET is done, SUCCESS will be returned. If the input is not a valid MorseChar
 * then the internal state should be reset and STANDARD_ERROR should be returned.
 * 
 * @param in A value from the MorseChar enum which specifies how to traverse the Morse tree.
 * 
 * @return Either SUCCESS on DECODE_RESET or when the next traversal location is still a valid
 *         character, the decoded character on END_OF_CHAR, or STANDARD_ERROR if the Morse tree
 *         hasn't been initialized, the next traversal location doesn't exist/represent a character,
 *         or `in` isn't a valid member of the MorseChar enum.
 */
char MorseDecode(MorseChar in){
    
    if(in == MORSE_CHAR_DOT){
        if(morse != NULL){
            morse = morse->leftChild;
        }
        return SUCCESS;
    }
    else if (in == MORSE_CHAR_DASH){
        if(morse != NULL){
            morse = morse->rightChild;
        }
        return SUCCESS;
    }
    else if(in == MORSE_CHAR_END_OF_CHAR){
        if(morse == NULL){
            return '#';
        }
        else if(morse->data == '\0'){
            return '#'; 
        }
        else {
            return morse->data; 
        }
    }
    else if(in == MORSE_CHAR_DECODE_RESET){
        morse = morseRoot;
        return SUCCESS;
    }
    else {
        return STANDARD_ERROR;
    }
}

/**
 * This function calls ButtonsCheckEvents() once per call and returns which, if any,
 * of the Morse code events listed in the enum above have been encountered. It checks for BTN4
 * events in its input and should be called at 100Hz so that the timing works. The
 * length that BTN4 needs to be held down for a dot is >= 0.25s and < 0.50s with a dash being a button
 * down event for >= 0.5s. The button uptime various between dots/dashes (>= .5s), letters
 * (>= 1s), and words (>= 2s).
 *
 * @note This function assumes that the buttons are all unpressed at startup, so that the first
 *       event it will see is a BUTTON_EVENT_*DOWN.
 *
 * So pressing the button for 0.3s, releasing it for 0.3s, pressing it for 0.6s, and then waiting
 * will decode the string '.-' (A). It will trigger the following order of events:
 * MORSE_EVENT_NONEs, 1 MORSE_EVENT_DOT, more MORSE_EVENT_NONEs, a MORSE_EVENT_DASH, more
 * MORSE_EVENT_NONEs, a MORSE_EVENT_END_CHAR, and then MORSE_EVENT_INTER_WORDs.
 * 
 * @return The MorseEvent that occurred.
 */
MorseEvent MorseCheckEvents(void){
    //INITIALIZE buttonEvent
    buttonEvent = ButtonsCheckEvents();
    
    //RUNNING TIMER
    timer++;
    
    //FOLLOW STATE MACHINE OUTLINE IN LAB MANUAL!!!!!
    switch(morseState){
        case(WAITING): 
            //Check for button 4 event. Transition through short/long presses
            if(buttonEvent == BUTTON_EVENT_4DOWN){
                timer = 0;
                morseState = DOT;
            }
            else{
                morseState = WAITING;
            }
            break;
            
        case(DOT):
            //When in DOT, BTN4 could still be down -> DASH. 
            //Make if statements to ensure which press with BTN4UP
            //timer++;
            if(timer >= MORSE_EVENT_LENGTH_DOWN_DOT){
                morseState = DASH;
            } else if (buttonEvent == BUTTON_EVENT_4UP){
                morseState = INTER_LETTER;
                timer = 0;
                return MORSE_EVENT_DOT;
            } 
            return MORSE_EVENT_NONE;
            
            break;
            
        case(DASH):
            //time is irrelevant since dash is longest press.
            if(buttonEvent == BUTTON_EVENT_4UP){
                morseState = INTER_LETTER;
                timer = 0;
                return MORSE_EVENT_DASH;
            } else {
                morseState = DASH;
            }
            return MORSE_EVENT_NONE;
            break;
            
        case(INTER_LETTER):
            //timer++;
            //if button is not being pressed for a long time, it is end of word -> return
            if(timer >= MORSE_EVENT_LENGTH_UP_INTER_LETTER_TIMEOUT){
                morseState = WAITING;
                return MORSE_EVENT_INTER_WORD;
            }
            else if (buttonEvent == BUTTON_EVENT_4DOWN){
                if(timer >= MORSE_EVENT_LENGTH_UP_INTER_LETTER){
                    timer = 0;
                    morseState = DOT;
                    return MORSE_EVENT_INTER_LETTER;
                }
                else{
                    timer = 0;
                    morseState = DOT;
                    return MORSE_EVENT_NONE;
                }
            } 
            return MORSE_EVENT_NONE;
            break;
    }
    return MORSE_EVENT_NONE;
}




/* Prints Correct Tree
int main(){
    BOARD_Init();
    
    Node *test = TreeCreate(6, morseTree);
    PrintTree(test,0); 
    
    
    
    while(1);
}*/