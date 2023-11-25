#include "common.h"


// Read a char bit by bit using readBitFromFile(). Returns EOF if unsucessful.
int readCharFromFile(FILE *fp_in_file, char *byte);

// Reads a byte from a file and returns a bit of the byte on every call. Returns EOF if unsucessful.
int readBitFromFile(FILE *fp_in_file, char *bit);


int main()
{
    long num_characters_read;
    unsigned short int tree_size_read;
    node *root;

    FILE *fp_in_file = fopen("input_message.txt.huff", "r");

    fread(&num_characters_read, sizeof(num_characters_read), 1, fp_in_file);
    printf("chars read: %ld\n", num_characters_read);
    
    fread(&tree_size_read, sizeof(tree_size_read), 1, fp_in_file);
    printf("tree size: %d\n", tree_size_read);

    char bit;
    char byte;

    // 1t
    readBitFromFile(fp_in_file, &bit);
    printf("%c ", bit + '0');
    readCharFromFile(fp_in_file, &byte);
    printf("%c ", byte);

    // 1a
    readBitFromFile(fp_in_file, &bit);
    printf("%c ", bit + '0');
    readCharFromFile(fp_in_file, &byte);
    printf("%c ", byte);

    // 1r
    readBitFromFile(fp_in_file, &bit);
    printf("%c ", bit + '0');
    readCharFromFile(fp_in_file, &byte);
    printf("%c ", byte);

    //root = ReconstructHuffmanTree(fp_in_file);
    
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
            perror("Failed to read a byte to from input file!");
            return EOF;
        }
        remaining_bits = CHAR_BIT;
    }

    // Get the (remaining_bits-1)th bit of the byte (read the bits starting from MSB to LSB)
    remaining_bits--;
    *bit = (i_byte >> remaining_bits) & 1;

    return 0;
}
