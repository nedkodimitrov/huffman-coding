#include "common.h"


// Reconstruct the serialized Huffman tree in the header of the compressed file. Returns the root of the tree or NULL if unsuccessful.
node *ReconstructHuffmanTree(FILE *fp_in_file, unsigned short int tree_size);

// Decode an encoded message using the Huffman tree. Returns 0 if successful and EOF if unsucessful.
int writeDecodedContent(node *root, long decoded_file_size, FILE *fp_in_file, FILE *fp_out_file);

// Read a char bit by bit using readBitFromFile(). Returns EOF if unsucessful.
int readCharFromFile(FILE *fp_in_file, char *byte);

// Reads a byte from a file and returns a bit of the byte on every call. Returns EOF if unsucessful.
int readBitFromFile(FILE *fp_in_file, char *bit);


int main(int argc, char *argv[])
{
    char in_file_name[FILE_NAME_MAX_LENGTH + COMPRESSED_FILE_EXTENSION_LENGTH] = {'\0'};  // container for the name of the compressed input file
    char out_file_name[FILE_NAME_MAX_LENGTH - COMPRESSED_FILE_EXTENSION_LENGTH + 8] = "decoded_";  // The name of the output decoded file
    node *root;  // The root of the reconstructed Huffman tree
    FILE *fp_in_file = NULL;  // File pointer for the input file
    FILE *fp_out_file = NULL;  // File pointer for the output file
    long decoded_file_size;  // The size of the unencoded input file (number of characters)
    unsigned short int tree_size; // number of nodes in the Huffman tree

    // Get the name of the file that will be decompressed from the CLA; Make sure it ends with COMPRESSED_FILE_EXTENSION (.huff)
    if ((getFileName(argc, argv, in_file_name, FILE_NAME_MAX_LENGTH + COMPRESSED_FILE_EXTENSION_LENGTH) == -1))
    {
        return INVALID_FILE_NAME;
    }

    if ((strlen(in_file_name) < COMPRESSED_FILE_EXTENSION_LENGTH + 1)
        || (strcmp(in_file_name + strlen(in_file_name) - COMPRESSED_FILE_EXTENSION_LENGTH + 1, COMPRESSED_FILE_EXTENSION) != 0))
    {
        printf("The input file must have %s extension\n", COMPRESSED_FILE_EXTENSION);
        return INVALID_FILE_NAME;
    }

    // Open the input .huff file that will be decoded
    fp_in_file = fopen(in_file_name, "r");
    if (fp_in_file == NULL)
    {
        printf("Failed to open the input file!\n");
        return FAIL_OPEN_INPUT_FILE;
    }

    // Read the size of the unencoded file and the size of the Huffman tree from the header of the compressed file.
    if (fread(&decoded_file_size, sizeof(decoded_file_size), 1, fp_in_file) < 1
        || fread(&tree_size, sizeof(tree_size), 1, fp_in_file) < 1)
    {
        printf("Failed to read the header of the input file!");
        fclose(fp_in_file);
        return FAIL_READ_HEADER;
    }

    // Reconstruct the Huffman tree from its serialized representation in the header of the comrpessed file.
    root = ReconstructHuffmanTree(fp_in_file, tree_size);
    if (root == NULL)
    {
        printf("Failed to create the Huffman tree!");
        fclose(fp_in_file);
        return FAIL_CREATE_HUFFMAN_TREE;
    }

    // Open the output file where the compressed content of input file will be stored
    // Remove the huffman extension from the name of the file
    strcat(out_file_name, in_file_name);
    out_file_name[strlen(out_file_name) - COMPRESSED_FILE_EXTENSION_LENGTH + 1] = '\0';
    fp_out_file = fopen(out_file_name, "w");
    if (fp_out_file == NULL)
    {
        printf("Failed to open the output file!\n");
        fclose(fp_in_file);
        freeBinaryTree(root);
        return FAIL_OPEN_OUTPUT_FILE;
    }

    // Write the decoded content of the input file into the output file
    if (writeDecodedContent(root, decoded_file_size, fp_in_file, fp_out_file) == EOF)
    {
        fclose(fp_in_file);
        fclose(fp_out_file);
        freeBinaryTree(root);
        return FAIL_READ_BODY;
    }

    printf("\nSuccessfully decoded %s into %s!\n", in_file_name, out_file_name);

    // Close opened file and free allocated memory
    fclose(fp_in_file);
    fclose(fp_out_file);
    freeBinaryTree(root);

    return 0;
}


// Reconstruct the serialized Huffman tree in the header of the compressed file. Returns the root of the tree or NULL if unsuccessful.
node *ReconstructHuffmanTree(FILE *fp_in_file, unsigned short int tree_size)
{
    char bit;
    char character;
    static priority_queue_element *stack_top = NULL;
    node *node1 = NULL, *node2 = NULL;
    node *root = NULL;

    // Read all tree nodes from the header of the file
    for (unsigned short int i = 0; i < tree_size; i++)
    {
        if (readBitFromFile(fp_in_file, &bit) == EOF)
        {
            freePriorityQueue(&stack_top);
            return NULL;
        }
        
        if (bit == 1)  // Leaves are denoted as 1 followed by a character (The characters are stored in the leaves of the Huffman tree).
        {
            // Push a new element to the stack
            if (readCharFromFile(fp_in_file, &character) == EOF || pushToPriorityQueue(&stack_top, character, 1, NULL, NULL) == -1)
            {
                freePriorityQueue(&stack_top);
                return NULL;
            }
        }
        else  // Parent nodes are denoted as 0
        {
            // Pop 2 elements from the stack and add a new one which is parent to them and has no character associated with it.
            node1 = popPriorityQueue(&stack_top);
            node2 = popPriorityQueue(&stack_top);
            if (node1 == NULL || node2 == NULL || pushToPriorityQueue(&stack_top, '\0', 1, node2, node1) == -1)
            {
                freePriorityQueue(&stack_top);
                return NULL;
            }
        }
    }

    // Pop the last remaining element, whose node is the root of the Huffman tree.
    root = popPriorityQueue(&stack_top);
    
    // If the stack is not empty, something went wrong.
    if (stack_top != NULL)
    {
        freePriorityQueue(&stack_top);
        return NULL;
    }

    return root;
}


// Decode an encoded message using the Huffman tree. Returns 0 if successful and EOF if unsucessful.
int writeDecodedContent(node *root, long decoded_file_size, FILE *fp_in_file, FILE *fp_out_file)
{
    node *trav = root; // Used to traverse the Huffman tree
    char bit;
    long characters_written = 0;

    // Follow the tree path from encoded_message
    while (characters_written < decoded_file_size)
    {
        if (readBitFromFile(fp_in_file, &bit) == EOF)
        {
            return EOF;
        }

        // if the code is '0', go to the left subtree, else to the right subtree.
        if (bit == 0)
        {
            trav = trav->left;
        }
        else
        {
            trav = trav->right;
        }

        // If we reached a leaf (the symbols are stored in the leafs. the other nodes have '\0' for their symbol), 
        // store its symbol in decoded_message and go back to the root of the Huffman tree.
        if (trav->left == NULL && trav->right == NULL)
        {
            if (fputc(trav->character, fp_out_file) == EOF)
            {
                printf("Failed to allocate memory for a symbol in the decoded message file!");
                return EOF;
            }
            trav = root;
            characters_written++;
        }
    }

    return 0;
}


// Read a char bit by bit using readBitFromFile(). Returns EOF if unsucessful.
int readCharFromFile(FILE *fp_in_file, char *byte)
{
    char bit;
    // Need to read it bit by bit so that it doesn't get read from the file before some other bits that have not filled a byte yet.
    for (int j = CHAR_BIT - 1; j >= 0; j--)
    {
        if (readBitFromFile(fp_in_file, &bit) == EOF)
        {
            return EOF;
        }

        *byte = ((*byte << 1) | bit);
    }
    return 0;
}


// Reads a byte from a file and returns a bit of the byte on every call. Returns EOF if unsucessful.
int readBitFromFile(FILE *fp_in_file, char *bit)
{
    static int i_byte;  // fgetc() returns an int so that it can represent every char + EOF
    static short int remaining_bits = 0;  // Counts how many bits of the byte have been read.

    // We can't read an individual bit from a file, but rather a whole byte.
    if (remaining_bits == 0)
    {
        if ((i_byte = fgetc(fp_in_file)) == EOF)
        {
            printf("Failed to read a byte from input file!");
            return EOF;
        }
        remaining_bits = CHAR_BIT;
    }

    // Get the (remaining_bits-1)th bit of the byte (read the bits starting from MSB to LSB)
    remaining_bits--;
    *bit = (i_byte >> remaining_bits) & 1;

    return 0;
}