

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define FILE_NAME_MAX_LENGTH 100
#define COMPRESSED_FILE_EXTENSION ".huff"
#define COMPRESSED_FILE_NAME_MAX_LENGTH (FILE_NAME_MAX_LENGTH + sizeof(COMPRESSED_FILE_EXTENSION))

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