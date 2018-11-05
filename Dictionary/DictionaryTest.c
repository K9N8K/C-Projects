/*
	DictionaryTest.c
	Kyle Ko
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Dictionary.h"

#define MAX_LEN 100

int main(int argc, char* argv[]){

	Dictionary test = newDictionary();

	// Test empty states
	printf("IsEmpty(): %d\n", isEmpty(test));
	printf("Size(): %d\n", size(test));

	// Insert
	insert(test,"key1", "value1");
	insert(test,"key2", "value2");
	insert(test,"key3", "value3");
	insert(test,"key4", "value4");
	printf("IsEmpty(): %d\n", isEmpty(test));
	printf("Size(): %d\n", size(test));
	printf("lookup key 1: %s", lookup(test, "key1"));
	printf("lookup key 2: %s", lookup(test, "key2"));
	printf("lookup key 3: %s", lookup(test, "key3"));
	printf("lookup key 4: %s", lookup(test, "key4"));
	printDictionary(stdout,test);

	// Delete
	delete(test, "key3");
	printf("Size(): %d\n", size(test));

	makeEmpty(test);
	printf("IsEmpty(): %d\n", isEmpty(test));
	printf("Size(): %d\n", size(test));

	freeDictionary(test);


	// Error Checks
	//printf("lookup key 5: %s", lookup(test, "key5"));


}