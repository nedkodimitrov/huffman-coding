/*
 * Functions related to the data structures in common.h
 * used both in encode and decode
*/


#include "common.h"


int getFileName(int argc, char *argv[], char *filename, size_t max_length)
{
    if (argc == 2)
    {
        if (strlen(argv[1]) > max_length)
        { 
            printf("File name is too long!");
            return -1;
        }
        strcpy(filename, argv[1]);
    }
    else
    {
        printf("Usage: %s <file name>\n", argv[0]);
        return -1;
    }

    return 0;
}


// Recursively free memory used by the Huffman tree
void freeBinaryTree(node *root)
{
    if (root)
    {
        freeBinaryTree(root->left);
        freeBinaryTree(root->right);
        free(root);
    }
}


// Create a new Huffman tree node
node *createNode(char character, int frequency, node *left, node *right)
{
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL)
    {
        printf("Failed to allocate memory for a tree node!\n");
        return NULL;
    }
    new_node->character = character;
    new_node->left = left;
    new_node->right = right;
    new_node->frequency = frequency;
    return new_node;
}


// Create a new priority queue element
priority_queue_element *createPriorityQueueElement(char character, int frequency, node *left, node *right)
{
    priority_queue_element *new_element = malloc(sizeof(priority_queue_element));
    if (new_element == NULL)
    {
        printf("Failed to allocate memory for a priority queue element!\n");
        return NULL;
    }

    new_element->pnode = createNode(character, frequency, left, right);
    if (new_element->pnode == NULL)
    {
        return NULL;
    }

    new_element->next = NULL;

    return new_element;
}


// Push a new element to the top of the priority queue. Returns 0 if successful and -1 if unsuccessful.
int pushToPriorityQueue(priority_queue_element **p_priority_queue, char character, int frequency, node *left, node *right)
{
    priority_queue_element *new_element = createPriorityQueueElement(character, frequency, left, right);
    if (new_element == NULL)
    {
        return -1;
    }

    new_element->next = *p_priority_queue;
    *p_priority_queue = new_element;

    return 0;
}


// Function to insert a node into the priority queue into the correct position according to its frequency. Returns 0 if successful and -1 if unsuccessful.
int insertIntoPriorityQueue(priority_queue_element **p_priority_queue, char character, int frequency, node *left, node *right)
{
    priority_queue_element *next_node = *p_priority_queue;
    priority_queue_element *previous_node = NULL;
    priority_queue_element *new_element = createPriorityQueueElement(character, frequency, left, right);
    
    if (new_element == NULL)
    {
        return -1;
    }

    while (next_node && next_node->pnode->frequency <= frequency)
    {
        previous_node = next_node;
        next_node = next_node->next;
    }

    // If previous_node is still NULL, the new node should be the head of the list
    if (previous_node == NULL)
    {
        new_element->next = *p_priority_queue;
        *p_priority_queue = new_element;
    }
    else
    {
        new_element->next = next_node;
        previous_node->next = new_element;
    }

    return 0;
}


// Pop an element from the priority queue
node *popPriorityQueue(priority_queue_element **p_priority_queue)
{
    node *node = NULL;
    if (*p_priority_queue)
    {
        priority_queue_element *temp = *p_priority_queue;
        node = (*p_priority_queue)->pnode;
        
        *p_priority_queue = (*p_priority_queue)->next;
        
        free(temp);
    }

    return node;
}

// Free priority queue and its nodes. Used when failed to allocate memory for a new element and must exit the program.
void freePriorityQueue(priority_queue_element **p_priority_queue)
{
    node *temp = NULL;
    // Free the current priority queue element and go to the next until reach NULL
    do
    {
        temp = popPriorityQueue(p_priority_queue);
        freeBinaryTree(temp);
    }
    while (temp != NULL);
}