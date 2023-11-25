#include "common.h"


// Number of ASCII characters. Used to determine the size of frequency_table and encoded_characters_table
#define NUM_ASCII 256
// Max length of the huffman code for a single character (im not quite sure what it should be, maybe needs to be set to the height of the tree.)
#define MAX_ENCODED_CHARACTER_LENGTH 20


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
*  which the size of the input file, the size of the Huffman tree and the serialized Huffman tree.
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


int main(int argc, char *argv[])
{
    char file_name[COMPRESSED_FILE_NAME_MAX_LENGTH] = {'\0'};  // container for the names of the input file and the output file
    FILE *fp_in_file = NULL;  // File pointer for the input file
    FILE *fp_out_file = NULL;  // File pointer for the output file
    node *root = NULL;  // The root of the Huffman tree
    /*
    * Table to store characters and their Huffman binary codes 
    * first dimension corresponds to ASCII character, second dimension is the encoded character (the path in the Huffman tree)
    * e.g. encoded_characters_table['a'] = "001"
    * used because otherwise would have to blindly traverse the whole tree for every character when compressing the input file.
    */
    char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH] = { [0 ... NUM_ASCII - 1] = { '\0' } };
    unsigned short int tree_size; // number of nodes in the Huffman tree
    long in_file_size; // size of the input file - how many characters it contains

    // Get the name of the file that will be compressed from the CLA
    if (getFileName(argc, argv, file_name, FILE_NAME_MAX_LENGTH) == -1)
    {
        return -1;
    }
    // Read the content of the input file into a string
    fp_in_file = fopen(file_name, "r");
    if (fp_in_file == NULL)
    {
        perror("Failed to open the input file!\n");
        return -1;
    }

    // Create the Huffman tree of the input file content
    root = createHuffmanTree(fp_in_file);
    if (root == NULL)
    {
        fclose(fp_in_file);
        return -1;
    }

    // Store the huffman codes for each character in a table
    tree_size = populateEncodedCharactersTable(root, 0, encoded_characters_table);

    // Open the output file where the compressed content of input file will be stored
    strcat(file_name, COMPRESSED_FILE_EXTENSION);
    fp_out_file = fopen(file_name, "w");
    if (fp_out_file == NULL)
    {
        perror("Failed to open the output file!\n");
        fclose(fp_in_file);
        freeBinaryTree(root);
        return -1;
    }

    // Write the header of the compressed file
    in_file_size = ftell(fp_in_file);
    if (writeHeader(fp_out_file, in_file_size, tree_size, root) == EOF)
    {
        fclose(fp_in_file);
        fclose(fp_out_file);
        freeBinaryTree(root);
        return -1;
    }
    
    // Write the encoded content of the input file into the output file
    fseek(fp_in_file, 0, SEEK_SET);
    if (writeEncodedFileContent(encoded_characters_table, fp_in_file, fp_out_file) == EOF)
    {
        fclose(fp_in_file);
        fclose(fp_out_file);
        freeBinaryTree(root);
        return -1;
    }

    printf("Compression ratio = %.2lf%%\n", (double) ftell(fp_out_file) / in_file_size * 100);
    
    // Close opened file and free allocated memory
    fclose(fp_in_file);
    fclose(fp_out_file);
    freeBinaryTree(root);

    return 0;
}


// Create a Huffman tree from file content. Returns tree root or NULL if unsuccessful.
node *createHuffmanTree(FILE *fp_in_file)
{
    int frequency_table[NUM_ASCII] = {0}; // How many times each character is encountered in the file. E.g. frequency_table['a'] = 3
    priority_queue_element *priority_queue = NULL; // Priority queue where Huffman tree nodes are sorted by their character's frequency

    populateFrequencyTable(fp_in_file, frequency_table);

    priority_queue = frequencyTableToPriorityQueue(frequency_table);

    // Transform the priority queue into a Huffman tree and return the root of the tree
    return priorityQueueToHuffmanTree(&priority_queue);
}


// Populate a frequency table for a given file's content (how many times each character is encountered in the file)
void populateFrequencyTable(FILE *fp_in_file, int *frequency_table)
{
    char character;

    // Every time a character is encountered in the file, increment its frequency in the table
    while ((character = fgetc(fp_in_file)) != EOF)
    {
        frequency_table[(int)(character)]++; //  e.g. frequency_table['a']++
    }
}


// Create a priority queue from a frequency table (priority queue where Huffman tree nodes are sorted by their character's frequency).
// Returns the head of the queue or NULL if unsuccessful.
priority_queue_element *frequencyTableToPriorityQueue(int *frequency_table)
{
    priority_queue_element *priority_queue = NULL; // queue where Huffman tree nodes are sorted by their character's frequency

    // For every character that is encountered atleast once
    for (int i = 0; i < NUM_ASCII; i++)
    {
        if (frequency_table[i])
        {
            // Add a new element to the queue that contains a tree node of that character and the character's frequency
            if (insertIntoPriorityQueue(&priority_queue, (char)(i), frequency_table[i], NULL, NULL) == -1)
            {
                perror("Failed to create the priority queue from the frequency table!\n");
                freePriorityQueue(&priority_queue);
                return NULL;
            }
        }
    }

    return priority_queue;
}


// Transform a priority queue into a Huffman tree and free the memory used by the queue. Returns the root of the tree or NULL if unsuccesful.
node *priorityQueueToHuffmanTree(priority_queue_element **p_priority_queue)
{
    node *node1 = NULL, *node2 = NULL;
    
    // Sum all elements in the queue
    while (1)
    {
        // Pop the first two elements in the priority queue
        node1 = popPriorityQueue(p_priority_queue);
        node2 = popPriorityQueue(p_priority_queue);

        // If there was only one remaining element in the queue, return its tree node which is going to be the root of the Huffman tree.
        if (node2 == NULL)
        {
            return node1;
        }

        // Add a new element to priority_queue which is a parent to the first two and its node frequency is the sum of the two.
        if (insertIntoPriorityQueue(p_priority_queue, '\0', node1->frequency + node2->frequency, node1, node2) == -1)
        {
            perror("Failed to create the Hufman tree from the priority queue!\n");
            freePriorityQueue(p_priority_queue);
            return NULL;
        }
    }
}


// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table.
// Returns the total number of nodes in the tree, which is saved in the header of the compressed file, so that the tree can be reconstructed when decoding.
unsigned short int populateEncodedCharactersTable(node *root, int tree_level,
        char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH])
{
    static char character_code[MAX_ENCODED_CHARACTER_LENGTH] = {'\0'}; // Keeps track of the path in the tree to the character (which is how the character is encoded).
    unsigned short int num_nodes = 0; // total number of nodes in the tree

    if (root)
    {
        num_nodes ++;

        // Write 0 to the path to the leaf when going to the left subtree
        character_code[tree_level] = '0';
        num_nodes += populateEncodedCharactersTable(root->left, tree_level + 1, encoded_characters_table);

        // Write 1 to the path to the leaf when going to the right subtree
        character_code[tree_level] = '1';
        num_nodes += populateEncodedCharactersTable(root->right, tree_level + 1, encoded_characters_table);

        if (root->left == NULL && root->right == NULL)
        {
            // The characters are stored in the leaves. Store the path to the leaf in the coresponding row of encoded_characters_table. 
            // E.g. encoded_characters_table['a'] = "001"
            character_code[tree_level] = '\0';
            strcpy(encoded_characters_table[(int)root->character], character_code);
            printf("Character:%c, Encoded:%s\n", root->character, character_code);
        }
    }

    return num_nodes;
}

/*
*  Write the header of the compressed file, needed when decoding it,
*  which the size of the input file, the size of the Huffman tree and the serialized Huffman tree.
*  Returns EOF if unsucessful.
*/
int writeHeader(FILE *fp_out_file, long in_file_size, unsigned short int tree_size, node *root)
{
    if ((fwrite(&in_file_size, sizeof(in_file_size), 1, fp_out_file) != 1) ||
        (fwrite(&tree_size, sizeof(tree_size), 1, fp_out_file) != 1) ||
        writeSerializedHuffmanTreeToFile(root, fp_out_file) == EOF)
    {
        perror("Failed to write the header of the compressed file!\n");
        return EOF;
    }

    return 0;
}


// Recursively traverse the Huffman tree and write it as serialized into a file. Returns EOF if unsucessful.
int writeSerializedHuffmanTreeToFile(node *root, FILE *fp_out_file)
{
    if (root)
    {
        writeSerializedHuffmanTreeToFile(root->left, fp_out_file);
        writeSerializedHuffmanTreeToFile(root->right, fp_out_file);

        if (root->left == NULL && root->right == NULL)
        {
            // The characters are stored in the leaves. For a leaf write 1 followed by its character
            if ((writeBitToFile(fp_out_file, 1) == EOF) || writeCharToFile(fp_out_file, root->character) == EOF)
            {
                return EOF;
            }
        }
        else
        {
            // For a parent node write 0
            if (writeBitToFile(fp_out_file, 0) == EOF)
            {
                return EOF;
            }
        }
    }
    return 0;
}


// Encode a file using the Huffman tree built from it. Returns EOF if unsucessful.
int writeEncodedFileContent(char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH], FILE *fp_in_file,
                            FILE *fp_out_file)
{
    char character;
    // Write the encoding of each character into the compressed file.
    while ((character = fgetc(fp_in_file)) != EOF)
    {
        for (int i = 0, len = strlen(encoded_characters_table[(int)character]); i < len; i++)
        {
            if (writeBitToFile(fp_out_file, encoded_characters_table[(int)character][i] - '0') == EOF)
            {
                perror("Failed to write the encoded content!\n");
                return EOF;
            }
        }
    }

    // Write seven 0 bits to make sure the last byte is complete.
    for (int i = 0; i < CHAR_BIT - 1; i++)
    {
        if (writeBitToFile(fp_out_file, 0) == EOF)
        {
            perror("Failed to write the last byte!\n");
            return EOF;
        }
    }
    
    return 0;
}


// Write a char bit by bit using writeBitToFile(). Returns EOF if unsucessful.
int writeCharToFile(FILE *fp_out_file, char byte)
{
    // Need to write it bit by bit so that it doesn't get saved to the file before some other bits that have not filled a byte yet.
    for (int j = CHAR_BIT - 1; j >= 0; j--)
    {
        if (writeBitToFile(fp_out_file, (byte >> j) & 1) == EOF)
        {
            return EOF;
        }
    }
    return 0;
}


// After CHAR_BIT (8) bits have been accumulated, write a byte to the file. Returns EOF if unsucessful.
int writeBitToFile(FILE *fp_out_file, char bit)
{
    static unsigned char byte = 0;
    static short int bits = 0;

    // Add the new bit to the other bits of the previous calls of the function.
    byte = (byte << 1) | bit;
    bits++;

    // We can't write an individual bit to a file, but rather a whole byte.
    if (bits == CHAR_BIT)
    {
        if (fputc(byte, fp_out_file) == EOF)
        {
            perror("Failed to write a byte to the output file!");
            return EOF;
        }

        bits = 0;
        byte = 0;
    }

    return 0;
}