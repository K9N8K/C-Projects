// **** Include libraries here ****

// Standard libraries
#include <stdio.h>
#include <string.h>

//CMPE13 Support Library
#include "BOARD.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Ascii.h"
#include "Adc.h"
#include "Buttons.h"
#include "Leds.h"


// Microchip libraries
#include <xc.h>
#include <plib.h>



// **** Set any macros or preprocessor directives here ****
// Set a macro for resetting the timer, makes the code a little clearer.
#define TIMER_2HZ_RESET() (TMR1 = 0)
#define topOn "|\01\01\01\01\01\01|"
#define topOff "|\02\02\02\02\02\02|"
#define bottomOn "|\03\03\03\03\03\03|"
#define bottomOff "|\04\04\04\04\04\04|"
#define middleClear "|      |"
#define middleLined "|------|"
#define longpress 450
#define timerefresh 0.002263

// **** Declare any datatypes here ****

void OledPrintBake(int time, int temperature, int on, int inputPoint);
void OledPrintToast(int time, int on);
void OledPrintBroil(int time, int on);

typedef enum {
    RESET,
    START,
    COUNTDOWN,
    PENDING_SELECTOR_CHANGE,
    PENDING_RESET,
    COUNTDOWN_FINISH,
    SCREEN_INVERT,
} state;

typedef enum {
    BAKE,
    TOAST,
    BROIL,
} cookType;

typedef enum {
    TIME,
    TEMP,
} input;

typedef struct ovenData {
    uint16_t temperature; // Temp in F
    cookType cookMode; // Cooking Mode
    state ovenState; // What state machine is in
    input inputSelection; // Whether the pot affects time or temp
    uint16_t runningTime; // used in Timer, used to check for LONG PRESS
} ovenData;

// **** Define any module-level, global, or external variables here ****
static char display[] = {};
static ovenData toaster;
static int temp, time, buttonFirst, buttonDownTime, onOff, flag, flashOn, timedFlash;
static double initTime;
static cookType tempMode;
static state tempState;
static input tempInput;
static uint8_t buttonEvent, ledMask; // used in Timer, = to ButtonsCheckEvents()

int main()
{
    BOARD_Init();

    // Configure Timer 1 using PBCLK as input. We configure it using a 1:256 prescalar, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR1 to F_PB / 256 / 2 yields a 0.5s timer.
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, BOARD_GetPBClock() / 256 / 2);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T1);
    INTSetVectorPriority(INT_TIMER_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T1, INT_ENABLED);

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

    // Configure Timer 3 using PBCLK as input. We configure it using a 1:256 prescalar, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR3 to F_PB / 256 / 5 yields a .2s timer.
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_256, BOARD_GetPBClock() / 256 / 5);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T3);
    INTSetVectorPriority(INT_TIMER_3_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_3_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T3, INT_ENABLED);

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/

    OledInit();
    AdcInit();
    LEDS_INIT();
    ButtonsInit();
    tempState = START;
    tempInput = TIME;
    tempMode = BAKE;
    temp = 350;
    onOff = 0;


    while (1) {
        toaster.ovenState = tempState;
        toaster.inputSelection = tempInput;
        toaster.cookMode = tempMode;

        switch (toaster.ovenState) {
        case(RESET):
            timedFlash = 0;
            onOff = 0;
            tempState = START;
            break;

        case(START):
            if (BUTTON_STATES() == BUTTON_STATE_3) {
                printf("BUTTON 3 DOWN\n");
                buttonFirst = toaster.runningTime;
                buttonDownTime = buttonFirst;
                while (BUTTON_STATES() == BUTTON_STATE_3) {
                    buttonDownTime++;
                    printf("%d\n", buttonDownTime);
                }
                tempState = PENDING_SELECTOR_CHANGE;
            }
            else if (buttonEvent == BUTTON_EVENT_4DOWN) {
                tempState = COUNTDOWN;
            }
            else if ((AdcChanged()) && toaster.inputSelection == TIME) {
                time = ((AdcRead() >> 2) + 1);

                if (toaster.cookMode == BAKE) {
                    OledPrintBake(time, temp, onOff, 0);
                } else if (toaster.cookMode == TOAST) {
                    OledPrintToast(time, onOff);
                } else if (toaster.cookMode == BROIL) {
                    OledPrintBroil(time, onOff);
                }
            }
            else if ((AdcChanged()) && toaster.inputSelection == TEMP) {
                temp = ((AdcRead() >> 2) + 300);
                OledPrintBake(time, temp, onOff, 1);
            }

            break;

        case(COUNTDOWN):
            printf("COUNTING DOWN!\n");
            ledMask = 0b11111111;
            LEDS_SET(ledMask);
            initTime = time;
            onOff = 1;
            flag = 0;

            if (toaster.cookMode == BAKE) {
                while (initTime >= 0) {
                    if (initTime <= ((.125) * time)) {
                        ledMask = 0b10000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.25) * time)) {
                        ledMask = 0b11000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.375) * time)) {
                        ledMask = 0b11100000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.5) * time)) {
                        ledMask = 0b11110000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.625) * time)) {
                        ledMask = 0b11111000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.75) * time)) {
                        ledMask = 0b11111100;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.875) * time)) {
                        ledMask = 0b11111110;
                        LEDS_SET(ledMask);
                    }

                    if (buttonEvent & BUTTON_EVENT_4DOWN) {
                        buttonEvent = 0;
                        tempState = PENDING_RESET;
                        flag = 1;
                        break;

                    } else {
                        flag = 0;
                    }

                    initTime -= timerefresh; // HZ Converted To Seconds
                    OledPrintBake((int) initTime, temp, onOff, 0);
                }
            } else if (toaster.cookMode == TOAST) {
                while (initTime >= 0) {
                    if (initTime <= ((.125) * time)) {
                        ledMask = 0b10000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.25) * time)) {
                        ledMask = 0b11000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.375) * time)) {
                        ledMask = 0b11100000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.5) * time)) {
                        ledMask = 0b11110000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.625) * time)) {
                        ledMask = 0b11111000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.75) * time)) {
                        ledMask = 0b11111100;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.875) * time)) {
                        ledMask = 0b11111110;
                        LEDS_SET(ledMask);
                    }

                    if (buttonEvent & BUTTON_EVENT_4DOWN) {
                        buttonEvent = 0;
                        tempState = PENDING_RESET;
                        flag = 1;
                        break;

                    } else {
                        flag = 0;
                    }
                    initTime -= timerefresh;
                    OledPrintToast((int) initTime, onOff);
                }
            } else if (toaster.cookMode == BROIL) {
                while (initTime >= 0) {
                    if (initTime <= ((.125) * time)) {
                        ledMask = 0b10000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.25) * time)) {
                        ledMask = 0b11000000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.375) * time)) {
                        ledMask = 0b11100000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.5) * time)) {
                        ledMask = 0b11110000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.625) * time)) {
                        ledMask = 0b11111000;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.75) * time)) {
                        ledMask = 0b11111100;
                        LEDS_SET(ledMask);
                    } else if (initTime <= ((.875) * time)) {
                        ledMask = 0b11111110;
                        LEDS_SET(ledMask);
                    }

                    if (buttonEvent & BUTTON_EVENT_4DOWN) {
                        buttonEvent = 0;
                        tempState = PENDING_RESET;
                        flag = 1;
                        break;

                    } else {
                        flag = 0;
                    }
                    initTime -= timerefresh;
                    OledPrintBroil((int) initTime, onOff);
                }
            }
            LEDS_SET(0b00000000);

            if (flag == 1) {
                tempState = PENDING_RESET;
                flag = 0;
            } else {
                tempState = COUNTDOWN_FINISH;
            }
            break;

        case(PENDING_SELECTOR_CHANGE):
            if (buttonDownTime - buttonFirst < longpress) {
                printf("SHORT PRESS\n");
                tempInput = TIME;
                if (toaster.cookMode == BAKE) {
                    tempMode = TOAST;
                    printf("BAKE -> TOAST\n");
                    OledPrintToast(time, onOff);
                } else if (toaster.cookMode == TOAST) {
                    tempMode = BROIL;
                    printf("TOAST -> BROIL\n");
                    OledPrintBroil(time, onOff);
                } else if (toaster.cookMode == BROIL) {
                    printf("BROIL -> BAKE\n");
                    tempMode = BAKE;
                    OledPrintBake(time, temp, onOff, 0);
                }
            }
            else if (buttonDownTime - buttonFirst >= longpress) {
                printf("LONG PRESS\n");
                if (toaster.inputSelection == TIME) {
                    tempInput = TEMP;
                    tempState = START;
                    OledPrintBake(time, temp, onOff, 1);
                    continue;
                } else {
                    tempInput = TIME;
                    tempState = START;
                    OledPrintBake(time, temp, onOff, 0);
                    continue;
                }
            }
            tempState = START;
            break;

        case(PENDING_RESET):
            flag = 0;

            buttonFirst = toaster.runningTime;
            buttonDownTime = buttonFirst;
            while (BUTTON_STATES() == BUTTON_STATE_4) {
                buttonDownTime++;
                printf("%d   %d\n", buttonDownTime, buttonFirst);
            }
            if ((buttonDownTime - buttonFirst) > longpress) {
                tempState = RESET;
                printf("LONG\n");
            } else {
                printf("SHORT -> COUNTDOWN\n");
                tempState = COUNTDOWN;
            }
            break;

        case(COUNTDOWN_FINISH):
            if (flashOn == 1) {
                if (buttonEvent == BUTTON_EVENT_4DOWN) {
                    buttonEvent = 0;
                    flashOn = 0;
                }

                timedFlash++;
                printf("%d\n", timedFlash);
                if (timedFlash % 401 == 0) {
                    OledSetDisplayInverted();
                } else if (timedFlash % 803 == 0) {
                    OledSetDisplayNormal();
                }

                //timedFlash = 0;
                tempState = COUNTDOWN_FINISH;
                break;
            } else {
                tempState = SCREEN_INVERT;
            }
            break;

        case(SCREEN_INVERT):
            OledSetDisplayNormal();
            if (toaster.cookMode == BAKE) {
                OledPrintBake(time, temp, onOff, 0);
            } else if (toaster.cookMode == TOAST) {
                OledPrintToast(time, onOff);
            } else {
                OledPrintBroil(time, onOff);
            }

            tempState = RESET;
            break;

        default:
            break;
        }
    }

    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/
    while (1);
}

void OledPrintBake(int time, int temperature, int on, int inputPoint)
{
    if (on == 1) {
        sprintf(display, "%s  %s %s\n%s  %s %d:%.2d\n%s  %s %d\n%s",
                topOn, "Mode:", "Bake",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined, "Temp:", temp,
                bottomOn);
    }        // IF TIME IS SELECTED
    else if (inputPoint == 0) {
        sprintf(display, "%s  %s %s\n%s %s %d:%.2d\n%s  %s %d\n%s",
                topOff, "Mode:", "Bake",
                middleClear, ">Time:", (int) (time / 60), (time % 60),
                middleLined, "Temp:", temp,
                bottomOff);
    } else {
        sprintf(display, "%s  %s %s\n%s  %s %d:%.2d\n%s %s %d\n%s",
                topOff, "Mode:", "Bake",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined, ">Temp:", temp,
                bottomOff);

    }
    OledClear(OLED_COLOR_BLACK);
    OledDrawString(display);
    OledUpdate();
}

void OledPrintToast(int time, int on)
{
    if (on == 1) {
        sprintf(display, "%s  %s\n%s  %s %d:%.2d\n%s\n%s",
                topOn, "Mode: Toast",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined,
                bottomOn);
    } else {
        sprintf(display, "%s  %s\n%s  %s %d:%.2d\n%s\n%s",
                topOff, "Mode: Toast",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined,
                bottomOff);
    }
    OledClear(OLED_COLOR_BLACK);
    OledDrawString(display);
    OledUpdate();
}

void OledPrintBroil(int time, int on)
{
    if (on == 1) {
        sprintf(display, "%s  %s\n%s  %s %d:%.2d\n%s  %s\n%s",
                topOn, "Mode: Broil",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined, "Temp: 500",
                bottomOn);
    } else {
        sprintf(display, "%s  %s\n%s  %s %d:%.2d\n%s  %s\n%s",
                topOff, "Mode: Broil",
                middleClear, "Time:", (int) (time / 60), (time % 60),
                middleLined, "Temp: 500",
                bottomOff);
    }
    OledClear(OLED_COLOR_BLACK);
    OledDrawString(display);
    OledUpdate();
}

void __ISR(_TIMER_1_VECTOR, ipl4auto) TimerInterrupt2Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 4;

    if (onOff == 1) {
        flashOn = 1;
    } else {
        flashOn = 0;
    }

}

void __ISR(_TIMER_3_VECTOR, ipl4auto) TimerInterrupt5Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 12;
    toaster.runningTime++;

}

void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;
    buttonEvent = ButtonsCheckEvents();
}
