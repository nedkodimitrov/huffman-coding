#include "common.h"


// Number of ASCII characters. Used to determine the size of frequency_table and encoded_characters_table
#define NUM_ASCII 256
// Max length of the huffman code for a single character
// (probably can be optimized)
#define MAX_ENCODED_CHARACTER_LENGTH 64


// Create a Huffman tree from file content. Returns tree root or NULL if unsuccessful.
node *createHuffmanTree(FILE *fp_in_file);

// Populate a frequency table for a given file's content (how many times each character is encountered in the file)
void populateFrequencyTable(FILE *fp_in_file, int *frequency_table);

// Create a priority queue from a frequency table (priority queue where Huffman tree nodes are sorted by their character's frequency).
// Returns the head of the queue or NULL if unsuccessful.
priority_queue_element *frequencyTableToPriorityQueue(int *frequency_table);

// Transform a priority queue into a Huffman tree and free the memory used by the queue. Returns the root of the tree or NULL if unsuccesful.
node *priorityQueueToHuffmanTree(priority_queue_element **p_priority_queue);

// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table.
// Returns the total number of nodes in the tree, which is saved in the header of the compressed file, so that the tree can be reconstructed when decoding.
unsigned short int populateEncodedCharactersTable(node *root, int tree_level,
        char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH]);

/*
*  Write the header of the compressed file, needed when decoding it,
*  includes the size of the input file, the size of the Huffman tree and the serialized Huffman tree.
*  Returns EOF if unsucessful.
*/
int writeHeader(FILE *fp_out_file, long in_file_size, unsigned short int tree_size, node *root);

// Recursively traverse the Huffman tree and write it as serialized into a file. Returns EOF if unsucessful.
int writeSerializedHuffmanTreeToFile(node *root, FILE *fp_out_file);

// Encode a file using the Huffman tree built from it. Returns EOF if unsucessful.
int writeEncodedFileContent(char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH], FILE *fp_in_file,
                            FILE *fp_out_file);

// After CHAR_BIT (8) bits have been accumulated, write a byte to the file. Returns EOF if unsucessful.
int writeBitToFile(FILE *fp_out_file, char bit);

// Write a char bit by bit using writeBitToFile(). Returns EOF if unsucessful.
int writeCharToFile(FILE *fp_out_file, char byte);