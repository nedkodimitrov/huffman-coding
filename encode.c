#include "common.h"


// Number of ASCII characters. Used to determine the size of frequency table and encoded characters table
#define NUM_ASCII 256
// Max length of the huffman code for a single character (im not quite sure what it should be, maybe needs to be set to the height of the tree.)
#define MAX_ENCODED_CHARACTER_LENGTH 20

// Element in the priority queue where Huffman tree nodes are sorted by their character's frequency
typedef struct priority_queue_element
{
    node *pnode;
    int frequency; // How many times the character is encountered
    struct priority_queue_element *next;
} priority_queue_element;

// Create a Huffman tree from file content. Returns tree root or NULL if unsuccessful.
node *createHuffmanTree(FILE *fp_in_file);

// Populate a frequency table for a given file's content (how many times each character is encountered in the file)
void populateFrequencyTable(FILE *fp_in_file, int *frequency_table);

// Create a priority queue from a frequency table (priority queue where Huffman tree nodes are sorted by their character's frequency). Returns the top of the queue.
priority_queue_element *frequencyTableToPriorityQueue(int *frequency_table);

// push a node into the priority queue in the correct position according to its frequency. Returns 0 if successful and -1 if unsucessful.
int pushToPriorityQueue(priority_queue_element **priority_queue, char character, int frequency, node *left, node *right);

// Find the correct position in the priority queue for a node based on its frequency. Returns the queue element after which to insert the new one.
priority_queue_element *findPositionInPriorityQueue(int frequency, priority_queue_element *priority_queue);

// Transform a priority queue into a Huffman tree and free the memory used by the queue. Returns tree root or NULL if unsuccesful.
node *priorityQueueToHuffmanTree(priority_queue_element **priority_queue);

// ree priority queue and its nodes. Used when failed to allocate memory for a new element and must exit the program.
void freePriorityQueue(priority_queue_element **priority_queue);

// Encode a file using the Huffman tree built from it. Returns 0 if successful and -1 if unsucessful.
int encode(node *root, long num_characters, FILE *fp_in_file, FILE *fp_out_file);

// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table
void populateEncodedCharactersTable(node *root, int tree_level,
                                    char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH]);

// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table
void serializeTree(node *root, FILE *fp_out_file);
int treeSize(node *root);
int writeToOutputFile(FILE *fp_out_file, char *string, int is_bit);
int writeBitToOutputFile(FILE *fp_out_file, char bit);


int main(int argc, char *argv[])
{
    char in_file_name[FILE_NAME_MAX_LENGTH] = {'\0'};  // Name of the file that will be compressed
    char out_file_name[COMPRESSED_FILE_NAME_MAX_LENGTH] = {'\0'};  // Name of the compressed file
    FILE *fp_in_file = NULL;
    FILE *fp_out_file = NULL;
    node *root = NULL;  // The root of the Huffman tree

    if (argc == 2)
    {
        if (strlen(argv[1]) > FILE_NAME_MAX_LENGTH)
        { 
            perror("File name is too long!");
            return -1;
        }
        strcpy(in_file_name, argv[1]);
    }
    else
    {
        perror("Usage: ./encode <file name>");
        return -1;
    }

    // Read the content of the input file into a string
    fp_in_file = fopen(in_file_name, "r");
    if (fp_in_file == NULL)
    {
        perror("Failed to open the input file!\n");
        return -1;
    }

    // Create the Huffman tree of the input file content
    root = createHuffmanTree(fp_in_file);
    if (root == NULL)
    {
        perror("Failed to create the Huffman tree of the input file content!\n");
        fclose(fp_in_file);
        return -1;
    }

    // open the output file where the compressed content of input file will be stored
    strcpy(out_file_name, in_file_name);
    strcat(out_file_name, COMPRESSED_FILE_EXTENSION);
    fp_out_file = fopen(out_file_name, "w");
    if (fp_out_file == NULL)
    {
        perror("Failed to open the input file!\n");
        fclose(fp_in_file);
        freeBinaryTree(root);
        return -1;
    }

    // Encode the contents of the input file and save them in the output file
    long num_characters = ftell(fp_in_file);
    fseek(fp_in_file, 0, SEEK_SET);
    encode(root, num_characters, fp_in_file, fp_out_file);

    //printf("Compression ratio = %.2ld", ftell(fp_out_file) / num_characters);

    //traversehuffmanTree(root);
    
    fclose(fp_in_file);
    fclose(fp_out_file);

    freeBinaryTree(root);
}


// Create a Huffman tree from file content. Returns tree root or NULL if unsuccessful.
node *createHuffmanTree(FILE *fp_in_file)
{
    int frequency_table[NUM_ASCII] = {0}; // How many times each character is encountered in the message
    priority_queue_element *priority_queue = NULL; // Priority queue where Huffman tree nodes are sorted by their character's frequency

    populateFrequencyTable(fp_in_file, frequency_table);

    priority_queue = frequencyTableToPriorityQueue(frequency_table);
    if (priority_queue == NULL)
    {
        perror("Failed to create the Priority queue!");
        return NULL;
    }

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
        frequency_table[(int)(character)]++;
    }
}

// Create a priority queue from a frequency table (priority queue where Huffman tree nodes are sorted by their character's frequency). Returns the head of the queue.
priority_queue_element *frequencyTableToPriorityQueue(int *frequency_table)
{
    priority_queue_element *priority_queue = NULL; // queue where Huffman tree nodes are sorted by their character's frequency

    // For every character that has a frequency grater than 0
    for (int i = 0; i < NUM_ASCII; i++)
    {
        if (frequency_table[i])
        {
            // Add a new element to the queue that contains a tree node of that character and the character's frequency
            if (pushToPriorityQueue(&priority_queue, (char)(i), frequency_table[i], NULL, NULL) == -1)
            {
                perror("Failed to create the priority queue from the frequency table!\n");
                freePriorityQueue(&priority_queue);
                return NULL;
            }
        }
    }

    return priority_queue;
}

// Push a node into the priority queue into the correct position according to its frequency. Returns 0 if successful and -1 if unsucessful.
int pushToPriorityQueue(priority_queue_element **priority_queue, char character, int frequency, node *left, node *right)
{
    priority_queue_element *position; // Where the new queue element will be inserted

    // Create a new queue element that contains a tree node of that character and the character's frequency
    node *new_node = malloc(sizeof(node));
    priority_queue_element *new_queue_element = malloc(sizeof(priority_queue_element));
    if (new_node == NULL || new_queue_element == NULL)
    {
        free(new_node);
        free(new_queue_element);
        perror("Failed to allocate memory for a priority queue element!\n");
        return -1;
    }
    new_node->character = character;
    new_node->left = left;
    new_node->right = right;
    new_queue_element->frequency = frequency;
    new_queue_element->pnode = new_node;

    // Insert the new element into the correct position in the queue according to its frequency
    position = findPositionInPriorityQueue(frequency, *priority_queue);
    if (position)
    {
        new_queue_element->next = position->next;
        position->next = new_queue_element;
    } 
    else
    {
        new_queue_element->next = *priority_queue;
        *priority_queue = new_queue_element;
    }

    return 0;
}

// Find the correct position in the priority queue for an element based on its frequency. Returns the queue element after which to insert the new one.
priority_queue_element *findPositionInPriorityQueue(int frequency, priority_queue_element *priority_queue)
{
    // Need to store the previous position when traversing the queue, because we can't go back.
    priority_queue_element *position = NULL;

    // Traverse the queue until we reach an element with greater or equal frequency
    while (priority_queue && priority_queue->frequency <= frequency)
    {
        position = priority_queue;
        priority_queue = priority_queue->next;
    }

    // Return the address of the element before the one we found
    return position;
}

// Transform a priority queue into a Huffman tree and free the memory used by the queue. Returns tree root or NULL if unsuccesful.
node *priorityQueueToHuffmanTree(priority_queue_element **priority_queue)
{
    priority_queue_element *temp; // used when freeing elements of priority_queue
    node *root = NULL; // The root of the Huffman tree that is going to be created

    while ((*priority_queue)->next)
    {
        // Add a new element to priority_queue which is a parent to the first two and its frequency is the sum of the two
        if (pushToPriorityQueue(priority_queue, '\0', (*priority_queue)->frequency + (*priority_queue)->next->frequency,
                                (*priority_queue)->pnode, (*priority_queue)->next->pnode) == -1)
        {
            perror("Failed to create the Hufman tree from the priority queue!\n");
            freePriorityQueue(priority_queue);
            return NULL;
        }

        // Free first two elements after they were summed
        temp = *priority_queue;
        *priority_queue = temp->next->next;
        free(temp->next);
        free(temp);
    }
    
    // Free the last remaining queue element and return it's node - the root of the Huffman tree
    root = (*priority_queue)->pnode;
    free(*priority_queue);
    return root;
}

// Free priority queue and its nodes. Used when failed to allocate memory for a new element and must exit the program.
void freePriorityQueue(priority_queue_element **priority_queue)
{
    priority_queue_element *temp;

    // Free the current priority queue element and go to the next until reach NULL
    while (*priority_queue)
    {
        temp = *priority_queue;
        *priority_queue = (*priority_queue)->next;
        freeBinaryTree(temp->pnode);
        free(temp);
    }
}

// Encode a file using the Huffman tree built from it. Returns 0 if successful and -1 if unsucessful.
int encode(node *root, long num_characters, FILE *fp_in_file, FILE *fp_out_file)
{
    // Table to store characters and their Huffman binary codes 
    // first dimension corresponds to ASCII character, second dimension is the encoded character (the path in the Huffman tree)
    // used because otherwise would have to blindly traverse the whole tree for every character in message
    char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH] = { [0 ... NUM_ASCII - 1] = { '\0' } };
    //char character_code[MAX_ENCODED_CHARACTER_LENGTH]; // used when encoding individual characters in populateEncodedCharactersTable()
    char character;

    populateEncodedCharactersTable(root, 0, encoded_characters_table);

    // write the encoded file header
    // Write the number of characters in the input file in the header of the compressed file
    fwrite(&num_characters, sizeof(num_characters), 1, fp_out_file);
    long tree_size = treeSize(root);
    fwrite(&tree_size, sizeof(long), 1, fp_out_file);
    serializeTree(root, fp_out_file);

    // Write the encoding of each character of message into the encoded_message_file
    while ((character = fgetc(fp_in_file)) != EOF)
    {
        writeToOutputFile(fp_out_file, encoded_characters_table[(int)character], 1);
    }

    writeToOutputFile(fp_out_file, "0000000", 1); // fill the last byte
    

    return 0;
}

// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table
void populateEncodedCharactersTable(node *root, int tree_level,
                                    char encoded_characters_table[NUM_ASCII][MAX_ENCODED_CHARACTER_LENGTH])

{
    static char character_code[MAX_ENCODED_CHARACTER_LENGTH] = {'\0'}; // keeps track of the path in the tree to the character (which is how the character is encoded)

    if (root)
    {
        // Write 0 in the path to the leaf when going to the left subtree
        character_code[tree_level] = '0';
        populateEncodedCharactersTable(root->left, tree_level + 1, encoded_characters_table);

        // Write 1 in the path to the leaf when going to the right subtree
        character_code[tree_level] = '1';
        populateEncodedCharactersTable(root->right, tree_level + 1, encoded_characters_table);

        // If we reached a leaf (the characters are stored in the leafs. the other nodes have '\0' for their character)
        if (root->left == NULL && root->right == NULL)
        {
            // Store the path to the leaf in the coresponding row of encoded_characters_table
            character_code[tree_level] = '\0';
            strcpy(encoded_characters_table[(int)root->character], character_code);
            printf("Character:%c,Encoded:%s\n", root->character, character_code); // FIXME remove
        }
    }
}

// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table
int treeSize(node *root)
{
    if (root)
    {
        return  1 + treeSize(root->left) + treeSize(root->right);
    }
    return 0;
}


// Recursively traverse the Huffman tree and encode characters and store their binary representation (path in the tree) in encoded_characters_table
void serializeTree(node *root, FILE *fp_out_file)
{
    if (root)
    {
        serializeTree(root->left, fp_out_file);
        serializeTree(root->right, fp_out_file);

        // If we reached a leaf (the characters are stored in the leafs. the other nodes have '\0' for their character)
        if (root->left == NULL && root->right == NULL)
        {
            writeToOutputFile(fp_out_file, "1", 1);
            writeToOutputFile(fp_out_file, &(root->character), 0);
            printf("1%c%x\n", root->character, root->character);
        }
        else
        {
            writeToOutputFile(fp_out_file, "0", 1);
            printf("0\n");
        }
    }
}

int writeToOutputFile(FILE *fp_out_file, char *string, int is_bit)
{
    printf("THIS %s, %d\n", string, is_bit);
    for (int i = 0, len = strlen(string); i < len; i++)
    {
        if (is_bit)
        {
            writeBitToOutputFile(fp_out_file, string[i]);
        }
        else
        {
            for (int j = CHAR_BIT - 1; j >= 0; j--)
            {
                writeBitToOutputFile(fp_out_file, (string[i] >> j) & 1);
                printf("%d\n", (string[i] >> j) & 1);
            }
        }
    }
    
    return 0;
}

int writeBitToOutputFile(FILE *fp_out_file, char bit)
{
    static char byte = '\0';
    static short int bits = 0;

    byte = (byte << 1) | bit;
    bits++;
    if (bits == CHAR_BIT)
    {
        if (fputc(byte, fp_out_file) == EOF)
        {
            perror("Failed to write a byte to the output file!");
            return -1;
        }
        bits = 0;
        byte = '\0';
    }

    return 0;
}