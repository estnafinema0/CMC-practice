#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h> 
#include <string.h>
#include <sys/stat.h>

typedef struct node{
    int32_t key;
    int32_t value;
    struct node *left;
    struct node *right;
} *PBSTNODE;

enum {NODESIZE = sizeof(struct node)};


typedef struct filenode{
    int32_t parent;
    int32_t key;
    int32_t value;
    int32_t left;
    int32_t right;
} *PFILENODE;

PBSTNODE createBST(PBSTNODE tree, int32_t key, int32_t value);
void deleteBST(PBSTNODE root);
void putInFile(int fd, PBSTNODE root);
PBSTNODE buildTreeFromFile(int fd, PFILENODE node_array);
void printInorderBST(PBSTNODE root);
int isKeyInTree(PBSTNODE root, int32_t key, int32_t value);
void changeExistingKey(PBSTNODE root, int32_t key, int32_t value, PFILENODE *node_array);
PBSTNODE insertBST(PBSTNODE root, int32_t key, int32_t value, PFILENODE *node_array, int parent, int file_size);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary_tree_file>\n", argv[0]);
        exit(1);
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }

    struct stat info;
    if(fstat(fd, &info) == 1) {
        perror("fstat"); 
        exit(1);
    }
    int file_size = info.st_size;
    PFILENODE node_array = malloc(sizeof(struct filenode) * 1024);
    file_size = (file_size - 16) / 16;
    
    PBSTNODE root = NULL;
    root = buildTreeFromFile(fd, node_array);
    if (root == NULL) {
        puts("Initial tree is empty."); 
        file_size = 0;
    } else { 
        printf("Initial tree.\nKey Value\n");
        printInorderBST(root);
        putchar('\n');
    }
    
    printf("\nInput pairs: <key> <value> to insert in to the BST:\n");

    int32_t key, value;
    int flag;
    while (scanf("%d %d", &key, &value) == 2) {
        flag = isKeyInTree(root, key, value);
        if (flag == 1){
            changeExistingKey(root, key, value, &node_array);
        }
        else if (flag == 0){
            file_size++;
            if (root == NULL) root = createBST(root, key, value);
            else {
                root = insertBST(root, key, value, &node_array, root->key, file_size);
            }
            node_array[file_size].key = key; 
            node_array[file_size].value = value;
            node_array[file_size].left = 0; 
            node_array[file_size].right = 0;
        }
    }


    printf("\nUpdated tree in sorted order:\n");
    printInorderBST(root);

    deleteBST(root);
    close(fd);
    fd = open(argv[1], O_WRONLY | O_TRUNC, 0666);
    for (int i = 0; i <= file_size; i++){
        write(fd, &(node_array[i].key), sizeof(int32_t));
        write(fd, &(node_array[i].value), sizeof(int32_t));
        write(fd, &(node_array[i].left), sizeof(int32_t));
        write(fd, &(node_array[i].right), sizeof(int32_t));
    }
    free(node_array);
    close(fd);
    return 0;
}

PBSTNODE createBST(PBSTNODE node, int32_t key, int32_t value) 
{
    if (!node) {
        node = (PBSTNODE) malloc(NODESIZE);
        node->key = key;
        node->value = value;
        node->left = NULL;
        node->right = NULL;
    }
    else {
        if (node->key< key)
            node->right = createBST(node->right, key, value);
        else if (node->key > key)
            node->left = createBST(node->left, key, value);
    }
    return node;
}

void deleteBST(PBSTNODE root)
{
    if (root == NULL) return;
    deleteBST(root->left);
    deleteBST(root->right);
    free(root);
    root = NULL;
}

void putInFile(int fd, PBSTNODE root) {
    if (root == NULL) return;

    int32_t left_value = (root->left == NULL) ? 0 : root->left->key;
    int32_t right_value = (root->right == NULL) ? 0 : root->right->key;

    write(fd, &(root->key), sizeof(int32_t));
    write(fd, &(root->value), sizeof(int32_t));
    write(fd, &left_value, sizeof(int32_t));
    write(fd, &right_value, sizeof(int32_t));

    putInFile(fd, root->left);
    putInFile(fd, root->right);
}

PBSTNODE buildTreeFromFile(int fd, PFILENODE node_array){
    PBSTNODE current = NULL;
    int32_t key, value, right, left;
    int bytesCount = 0;
    int i = 0;
    while(read(fd, &key , sizeof(int32_t)) && read(fd, &value, sizeof(int32_t)) && read(fd, &left, sizeof(int32_t)) && read(fd, &right, sizeof(int32_t))){
        if (key == 0 && value == 0 && left == 0 && right == 0) 
        {
            node_array[i].key = key; 
            node_array[i].value = value; 
            node_array[i].left = left; 
            node_array[i].right = right;
            bytesCount = bytesCount + 16;
            lseek(fd, bytesCount, SEEK_SET);
            i++;
            continue;
        }
        current = createBST(current, key, value);
        node_array[i].key = key; 
        node_array[i].value = value; 
        node_array[i].left = left; 
        node_array[i].right = right;
        if (left != 0){
            lseek(fd, 16 * left, SEEK_SET);
            read(fd, &key , sizeof(int32_t));
            read(fd, &value, sizeof(int32_t));
            current = createBST(current, key, value);
        }
        if (right != 0){
            lseek(fd, 16 * right, SEEK_SET);
            read(fd, &key , sizeof(int32_t));
            read(fd, &value, sizeof(int32_t));
            current = createBST(current, key, value);
        }
        i++;
        bytesCount = bytesCount + 16;
        lseek(fd, bytesCount, SEEK_SET);
    }
    return current;
}

void printInorderBST(PBSTNODE root)
{
	if (root != NULL){
		printInorderBST(root->left);
		printf("%d   %d\n", root->key, root->value);
		printInorderBST(root->right);
	}
}

int isKeyInTree(PBSTNODE root, int32_t key, int32_t value)
{
    if (root == NULL) return 0; //Empty Tree
    if ((root->key == key) && (value != root->value)) 
        return 1;
    if ((root->key == key) && (value == root->value)) 
        return -1; 
    if (root->key > key) 
        return isKeyInTree(root->left, key, value);
    else 
        return isKeyInTree(root->right, key, value);
}

void changeExistingKey(PBSTNODE root, int32_t key, int32_t value, PFILENODE *node_array)
{
    int i = 0;
    if (root->key == key) {
        root->value = value; 
        while ((*node_array)[i].key != key)
        {
            i++; continue;
        }
        (*node_array)[i].value = value;
        return;
            }
    if (root->key > key) 
        changeExistingKey((root->left), key, value, node_array);
    else 
        changeExistingKey((root->right), key, value, node_array);
}

PBSTNODE insertBST(PBSTNODE root, int32_t key, int32_t value, PFILENODE *node_array, int parent, int file_size){
    if (root == NULL){
        root = createBST(root, key, value);
        int i = 0;
        while ((*node_array)[i].key != parent){
            i++;
            continue;
        }
        if (key < parent) 
            (*node_array)[i].left = file_size;
        else if(parent < key)
            (*node_array)[i].right = file_size;
    }
    if (key < root->key){
        root->left = insertBST(root->left, key, value, node_array, root->key, file_size);
    } else if (root->key < key){
        root-> right = insertBST(root->right, key, value, node_array, root->key, file_size);
    }
    return root;
}