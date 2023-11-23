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
node *createHuffmanTree(FILE *fp);

// Populate a frequency table for a given file's content (how many times each character is encountered in the file)
void populateFrequencyTable(FILE *fp, int *frequency_table);

// Create a priority queue from a frequency table (priority queue where Huffman tree nodes are sorted by their character's frequency). Returns the top of the queue.
priority_queue_element *frequencyTableToPriorityQueue(int *frequency_table);

// push a node into the priority queue in the correct position according to its frequency. Returns 0 if successful and -1 if unsucessful.
int pushToPriorityQueue(priority_queue_element **priority_queue, char character, int frequency, node *left, node *right);

// Find the correct position in the priority queue for a node based on its frequency. Returns the queue element after which to insert the new one.
priority_queue_element *findPositionInPriorityQueue(int frequency, priority_queue_element *priority_queue);

// Transform a priority queue into a Huffman tree and free the memory used by the queue. Returns tree root or NULL if unsuccesful.
node *priorityQueueToHuffmanTree(priority_queue_element **priority_queue);

// Free priority queue and its nodes. Used when failed to allocate memory for a new element and must exit the program.
void freePriorityQueue(priority_queue_element **priority_queue);


int main(int argc, char *argv[])
{
    char in_file_name[FILE_NAME_MAX_LENGTH] = {'\0'};  // Name of the file that will be compressed
    char out_file_name[COMPRESSED_FILE_NAME_MAX_LENGTH] = {'\0'};  // Name of the compressed file
    FILE *fp_in_file = NULL;
    FILE *fp_out_file = NULL;
    node *huffman_root = NULL;  // The root of the Huffman tree

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
    huffman_root = createHuffmanTree(fp_in_file);
    if (huffman_root == NULL)
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
        freeBinaryTree(huffman_root);
        return -1;
    }

    // Encode the contents of the input file and save them in the output file
    fseek(fp_in_file, 0, SEEK_SET);
    //encode(huffman_root, fp_in_file, fp_out_file);

    //printf("Compression ratio = %.2ld", ftell(fp_out_file) / ftell(fp_in_file));

    traversehuffmanTree(huffman_root);
    
    fclose(fp_in_file);
    fclose(fp_out_file);

    freeBinaryTree(huffman_root);
}


// Create a Huffman tree from file content. Returns tree root or NULL if unsuccessful.
node *createHuffmanTree(FILE *fp)
{
    int frequency_table[NUM_ASCII] = {0}; // How many times each character is encountered in the message
    priority_queue_element *priority_queue = NULL; // Priority queue where Huffman tree nodes are sorted by their character's frequency

    populateFrequencyTable(fp, frequency_table);

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
void populateFrequencyTable(FILE *fp, int *frequency_table)
{
    char character;

    // Every time a character is encountered in the file, increment its frequency in the table
    while ((character = fgetc(fp)) != EOF)
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