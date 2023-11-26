/*
 * Data structures, macros and function declarations
 * used both in encode and decode
*/


#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define FILE_NAME_MAX_LENGTH 100  // Max length of the name of the unencoded file
#define COMPRESSED_FILE_EXTENSION ".huff"  // the extension of the encoded file
#define COMPRESSED_FILE_EXTENSION_LENGTH sizeof(COMPRESSED_FILE_EXTENSION)  // length of the extension of the encoded file

// Error codes
#define INVALID_FILE_NAME 1
#define FAIL_OPEN_INPUT_FILE 2
#define FAIL_CREATE_HUFFMAN_TREE 3
#define FAIL_OPEN_OUTPUT_FILE 4
#define FAIL_WRITE_HEADER 5
#define FAIL_WRITE_BODY 6
#define FAIL_READ_HEADER 7
#define FAIL_READ_BODY 8


// Node in the Huffman tree
typedef struct node
{
    char character;
    int frequency; // How many times the character is encountered
    struct node *left, *right;
} node;

// Element in the priority queue where Huffman tree nodes are sorted by their character's frequency
typedef struct priority_queue_element
{
    node *pnode;
    struct priority_queue_element *next;
} priority_queue_element;

// Get the name of the file that will be compressed from the CLA
int getFileName(int argc, char *argv[], char *filename, size_t max_length);

// Recursively free memory used by the Huffman tree
void freeBinaryTree(node *root);

// Create a new Huffman tree node
node *createNode(char character, int frequency, node *left, node *right);

// Create a new priority queue element
priority_queue_element *createPriorityQueueElement(char character, int frequency, node *left, node *right);

// Push a new element to the top of the priority queue. Returns 0 if successful and -1 if unsuccessful.
int pushToPriorityQueue(priority_queue_element **p_priority_queue, char character, int frequency, node *left, node *right);

// insert a node into the priority queue in the correct position according to its frequency. Returns 0 if successful and -1 if unsucessful.
int insertIntoPriorityQueue(priority_queue_element **p_priority_queue, char character, int frequency, node *left, node *right);

// Pop an element from the priority queue
node *popPriorityQueue(priority_queue_element **p_priority_queue);

// ree priority queue and its nodes. Used when failed to allocate memory for a new element and must exit the program.
void freePriorityQueue(priority_queue_element **p_priority_queue);

#endif