// Heap size 2048 required!

// **** Include libraries here ****
// Standard libraries
#include <stdio.h>

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "BinaryTree.h"

// **** Set any macros or preprocessor directives here ****


// **** Define any module-level, global, or external variables here ****


// **** Declare any function prototypes here ****


int main()
{
    BOARD_Init();

    /******************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     *****************************************************************************/
    //[A B D E C F G]
    
    char testString[] = {'A','B','C'};
    char testString2[] = {'2', '7', 'E', '3', '9', 'Z', 'o'};
    
    Node *test = TreeCreate(2, testString);
    Node *test2 = TreeCreate(3, testString2);
    
    if(TreeCreate(2, testString) == NULL){
        printf("INVALID TREE: NOT PERFECT SIZE\n");
    } else {
        PrintTree(test,0);
        printf("\n\n");
        printf("Root's Left Child: %c\n", (GetLeftChild(test))->data);
        printf("Root's Right Child: %c\n", (GetRightChild(test))->data);
    }

    if(TreeCreate(3, testString2) == NULL){
        printf("INVALID TREE: NOT PERFECT SIZE\n");
    } else {
        PrintTree(test2,0);
        printf("\n\n");
        printf("Root's Left Child: %c\n", (GetLeftChild(test2))->data);
        printf("Root's Right Child: %c\n", (GetRightChild(test2))->data);
        
        Node *leftChild = (GetLeftChild(test2));
        Node *rightChild = (GetRightChild(test2));

        printf("Left Child's Left Child: %c\n", (GetLeftChild(leftChild))->data);
        printf("Left Child's Right Child: %c\n", (GetRightChild(leftChild))->data);
        printf("Right Child's Left Child: %c\n", (GetLeftChild(rightChild))->data);
        printf("Right Child's Right Child: %c\n", (GetRightChild(rightChild))->data);
    }
    
    
    
    /******************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     *****************************************************************************/

    while (1);
}

