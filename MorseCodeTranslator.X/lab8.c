
//CMPE13 Support Library
#include "BOARD.h"
#include "Oled.h"
#include "OledDriver.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Morse.h"
#include "BinaryTree.h"
#include "Buttons.h"

// **** Set any macros or preprocessor directives here ****
// Specify a bit mask for setting/clearing the pin corresponding to BTN4. Should only be used when
// unit testing the Morse event checker.
#define BUTTON4_STATE_FLAG (1 << 7)
#define WIDTH 21
#define OledRefresh() \
        sprintf(display,"%s\n%s", top, second); \
        OledDrawString(display); \
        OledUpdate();          
                     
// **** Define any module-level, global, or external variables here ****
static MorseEvent mEvents;
static int happen;
static char display[200];
static char top[WIDTH] = {};
static char second[WIDTH] = {};
static int windex;
static int index;


// **** Declare any function prototypes here ****
void ClearTop();


int main()
{
    BOARD_Init();


    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

/******************************************************************************
 * Your code goes in between this comment and the following one with asterisks.
 *****************************************************************************/
     
    OledInit();
    OledClear(OLED_COLOR_BLACK);
    char decodedChar;
    index = 0;
    windex = 0;
    happen = 0;
    
    //Check MorseInite())
    if(MorseInit() == STANDARD_ERROR){
        OledDrawString("MorseInit() Failed");
        OledUpdate();
        FATAL_ERROR();
    }
    
    //Running loop
    while(1){
        // DEFINED FUNCTION FOR APPENDING/REFRESHING OLED
        // State Machine for Morse Events
        // happen handles time repetitions. Only counts first, then reset by NO event
        switch(mEvents){
            case(MORSE_EVENT_DOT):
                if(happen == 0){
                    MorseDecode(MORSE_CHAR_DOT);
                    top[index] = '.';
                    index++;
                    happen++;
                    printf(".");
                }
                OledRefresh();
                break;
                
            case(MORSE_EVENT_DASH):
                if(happen == 0){
                    MorseDecode(MORSE_CHAR_DASH);
                    top[index] = '-';
                    index++;
                    happen++;
                    printf("-");
                }
                OledRefresh();
                break;
                
            case(MORSE_EVENT_NONE):
                happen = 0;
                break;
                
            case(MORSE_EVENT_INTER_LETTER):
                ClearTop();
                if(happen == 0){
                    //If decoded value is BLANK / NULL
                    if(MorseDecode(MORSE_CHAR_END_OF_CHAR) == '#'){
                        second[windex] = '#';
                    } else {
                        decodedChar = MorseDecode(MORSE_CHAR_END_OF_CHAR);
                        printf("%c", decodedChar);
                        second[windex] = decodedChar; 
                    }
                    windex++;
                    happen++;
                }
                MorseDecode(MORSE_CHAR_DECODE_RESET);
                OledRefresh();
                break;
            case(MORSE_EVENT_INTER_WORD):
                ClearTop();
                // If last element is space, no need for another space. Morse only uses on space
                if(second[windex - 1] == ' '){
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                }
                else if(happen == 0){
                    decodedChar = MorseDecode(MORSE_CHAR_END_OF_CHAR);
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                    second[windex] = ' ';
                    windex++; 
                    happen++;
                }
                
                OledRefresh();
                break;
        }
    }
  
/******************************************************************************
 * Your code goes in between this comment and the preceding one with asterisks.
 *****************************************************************************/

    while (1);
}

void ClearTop(){
    int i;
    for(i = 0; i <= index; i++){
        top[i] = ' ';
    } 
    OledRefresh();
    index = 0;
}



void __ISR(_TIMER_2_VECTOR, IPL4AUTO) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    //******** Put your code here *************//
    mEvents = MorseCheckEvents();
}
