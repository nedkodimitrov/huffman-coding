

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

// Recursively free memory used by the Huffman tree
void freeBinaryTree(node *root);

// Used for debugging
void traversehuffmanTree(node *root);

#endif