#include "common.h"

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

// Used for debugging
void traversehuffmanTree(node *root)
{
    if (root)
    {
        printf("%c\n", root->character);
        traversehuffmanTree(root->left);
        traversehuffmanTree(root->right);
    }
}