#include "common.h"


// Reconstruct the serialized Huffman tree in the header of the compressed file. Returns the root of the tree or NULL if unsuccessful.
node *ReconstructHuffmanTree(FILE *fp_in_file, unsigned short int tree_size);

// Decode an encoded file content using the Huffman tree. Returns 0 if successful and EOF if unsucessful.
int writeDecodedContent(node *root, long decoded_file_size, FILE *fp_in_file, FILE *fp_out_file);

// Read a char bit by bit using readBitFromFile(). Returns EOF if unsucessful.
int readCharFromFile(FILE *fp_in_file, char *byte);

// Reads a byte from a file and returns a bit of the byte on every call. Returns EOF if unsucessful.
int readBitFromFile(FILE *fp_in_file, char *bit);