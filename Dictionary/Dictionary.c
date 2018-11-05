/*
    Dictionary.c
    Kyle Ko
    Description: These functions define the header Dictionary.h
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include"Dictionary.h"

// NodeObj
typedef struct NodeObj{
    char* key;
    char* value;
    struct NodeObj* next;
} NodeObj;

// Node
typedef NodeObj* Node;

// newNode()
// constructor of the Node type
Node newNode(char* k, char* v){
    Node N = malloc(sizeof(NodeObj));
    assert(N!=NULL);
    N->key = k;
    N->value = v;
    N->next = NULL;
    return(N);
}

// freeNode();
// destructor for the Node type
void freeNode(Node* pN){
    if( pN != NULL && *pN != NULL ){
        free(*pN);
        *pN = NULL;
    }
}

// DictionaryObj
typedef struct DictionaryObj{
    Node head;
    Node tail;
    int numItems;
} DictionaryObj;


// newDictionary()
// constructor the the Dictionary type
Dictionary newDictionary(void){
    Dictionary D = malloc(sizeof(DictionaryObj));
    assert(D != NULL);
    D->head = NULL;
    D->tail = NULL;
    D->numItems = 0;
    return (D);
}

// destructor for the Dictionary type
void freeDictionary(Dictionary* pD){
    if( pD!=NULL && *pD!=NULL ){
        
        if(!isEmpty(*pD)){
            makeEmpty(*pD);
        }

        free(*pD);
        *pD = NULL;
    }
}

// Created this function because there is a 
// memory leak in makeEmpty() function
void freeAllNodes(Node Head){
    if(Head!=NULL){
        freeAllNodes(Head->next);
        freeNode(&Head);
    }
}

// isEmpty()
// returns 1 (true) if D is empty, 0 (false) otherwise
// pre: none
int isEmpty(Dictionary D){
    if( D == NULL ){
        fprintf(stderr, "Dictionary Error: calling isEmpty() on NULL Dictionary reference\n");
        exit(EXIT_FAILURE);
    }
    if(D->numItems > 0){
        return 0;
    }
    return 1;
}

// size()
// returns the number of (key, value) pairs in D
// pre: none
int size(Dictionary D){
    return D->numItems;
}

// lookup()
// returns the value v such that (k, v) is in D, or returns NULL if no 
// such value v exists.
// pre: none
char* lookup(Dictionary D, char* k){
    Node N = D->head;
    if( N == NULL ){
       fprintf(stderr, "Dictionary Error: calling lookup() on NULL Dictionary\n");
       exit(EXIT_FAILURE);
    }
    while(N != NULL){
        if(strcmp(N->key,k)== 0)
            return N->value;
        N = N->next;
    }
    return NULL;
}

// insert()
// pre condition: lookup(D, k) == NULL
void insert(Dictionary D, char* k, char* v){
    Node N = newNode(k, v);

    // Handle Empty Dictionary
    if( D->numItems == 0 ){
        D->head = D->tail = N;
    }
    // If already in dictionary
    else if( lookup(D, k) != NULL ){
        fprintf(stderr, "Dictionary Error: calling insert() on Duplicate Key\n");
        exit(EXIT_FAILURE);
    }
    else{
        D->tail->next = N;
        D->tail = N;
    }
    D->numItems++;
}

// delete()
// deletes pair with the key k
// pre: lookup(D, k)!=NULL
void delete(Dictionary D, char* k){
    
    Node N = D->head;
    
    if( lookup(D,k) == NULL ){
        fprintf(stderr, "Dictionary error: calling delete() on Non-existent Key\n");
        exit(EXIT_FAILURE);
    }
    // Single Item
    if( D->numItems < 2){
        D->head = NULL;
        D->numItems--; 
    } else if ( strcmp(N->key,k) == 0 ) {
        Node G = D->head;
        Node P = G->next;
        D->head = P;
        freeNode(&G);
        D->numItems--;
    } else {
        while(N !=NULL && N->next != NULL){
            if( strcmp(N->next->key, k) == 0){
                Node X = N;
                Node Y = N->next;
                X->next = Y->next;
                N = X;
                freeNode(&Y);
            }
            N=N->next;
         } 
         D->numItems--; 
      }     
 }

// makeEmpty()
// re-sets D to the empty state.
// pre: none
void makeEmpty(Dictionary D){
    freeAllNodes((D->head));
    D->head = NULL;
    D->tail = NULL;
    D->numItems = 0;
} 

// printDictionary()
// pre: none
// prints a text representation of D to the file pointed to by out
void printDictionary(FILE* out, Dictionary D){
    if(D == NULL){
        fprintf(stderr, "Dictionary Error: calling printDictionary() on NULL Dictionary\n");
        exit(EXIT_FAILURE);
    }
    for(Node N = D->head; N != NULL; N = N->next){
        fprintf(out, "%s %s \n", N->key, N->value);
    }
}

