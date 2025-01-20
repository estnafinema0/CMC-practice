#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"
#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"

enum{block_size = 4};

typedef struct node {
        char *data;
        struct node *next;
} *list;
  
char* getString(){
    char ch, *buffer;
    buffer = (char *)malloc(sizeof(char) * 4);  
    buffer[0]= '\0';
    int cur_length = 0;
    int overflow = 0; 
    while((ch = getchar ()) != '\n' && ch != ' ' && ch != EOF){
        if (overflow == cur_length) {
            buffer = (char *)realloc(buffer, sizeof(char) * (block_size + 4));
            if (buffer == NULL){
                printf(RED"Error: memory allocalion for buf is declined!\n"RESET);
                free(buffer); 
                return NULL; 
            }
            cur_length+=4;
        }
        buffer[overflow] = ch;
        overflow++;
    }
    if (overflow == cur_length){
        buffer = (char *)realloc(buffer, sizeof(char)*(block_size + 1));
        buffer[overflow]= '\0';
    }
    else buffer[overflow] = '\0'; 

    return buffer;
} 

list create_node(const char *string_data)
{
    list new_node = (list)malloc(sizeof(struct node));
    if (new_node == NULL){
        printf(RED"Error: memory allocalion for node is declined![1]\n"RESET);
        exit(1);
    }
    new_node->data = (char *)malloc(strlen(string_data) + 1);
    if (new_node->data == NULL){
        printf(RED"Error: memory allocalion for node's data is declined![1]\n"RESET);
        exit(2);
    }
    strcpy(new_node->data, string_data);
    new_node->next = NULL;
    return new_node;
}

list add_last_node(list head, const char *string_data)
{
    list new_node = create_node(string_data);
    if (head == NULL)
        return new_node;
    list current = head;
    while (current->next){
        current = current->next;
    }
    current->next = new_node;
    return head;
}

list build_list(void)
{
    list head = NULL;
    char string_data[120];
    printf(MAGENTA"Enter words with space to create list. \nTo end list enter with space before \".\":\n"RESET);
    while (1) {
        
        if (!scanf("%s", string_data)) 
            break;
        head = add_last_node(head, string_data);
    }
    return head;
}

list cleanise_list(list L){
    list removed_L = L, current;
    if  (!L) 
        return NULL;
    if (L->next == NULL) 
        return L;
    current = L->next;
    
    while ( (L->next)){
        if (strcmp(current->data, L->data)==0){
            while (strcmp(current->data, L->data) == 0 && current){
                L->next = current->next;
                free(current->data); 
                free(current);
                current = L->next;
                if (current==NULL) {
                    return removed_L;
                }
            }
        }
        else{
            L = L->next;
            if (L->next) {
                current = L->next;
            } else {
                return removed_L;
            }
        }
        
    }
    return removed_L;
}	

void delete_list(list head)
{
    list current;
    while (head){
       current = head;
        head = head->next;
        free(current->data);
        free(current);
    }
}

list print_list_forward(list head) {
    if (!head){
        printf(RED"Non existant data in list. Nothing to print![3]\n"RESET);
        return NULL;
    }    
    list current = head;
    while (current) {
        printf("%s ", current->data);
        current = current->next;
    }
    printf("\n");
    return head;
}

int dublicate_in_list(list L, const char* word){
    while (L){
        if (strcmp(L->data, word)==0) {
            return 1;
        }
        L = L->next;
    }
    return 0;
} 	

int sort_length(const char * a, const char * b)
{
    return strlen(a) - strlen(b);
}

int sort_althabetical(const char * a, const char * b)
{
    return strcmp(a, b);
}

int (*sort_criteria[])(const char * , const char * ) = {
    sort_length,
    sort_althabetical
};

void insert_node_sorted(list *head, const char * word, int(*createria)(const char * a, const char * b))
{
    list new_node = create_node(word);
    if (*head == 0 || createria(new_node->data, (*head)->data) <0){
        new_node->next = *head;
        *head = new_node;
    }else{
        list current = *head;
        while (current->next !=NULL && createria(new_node->data, current->next->data) > 0 ){
            current = current->next;
        }
        new_node->next= current->next;
        current->next = new_node;
    }
}

list merge_lists(list L1, list L2) 
{ 
    L1 = cleanise_list(L1);
    L2 = cleanise_list(L2);
    if (!L1 && !L2) 
        return NULL;
    else if (!L1) 
        return L2;
    else if (!L2)
        return L1;

    while (L1) {
        list temp = L1;
        L1 = L1->next;
        if (!dublicate_in_list(L2, temp->data)) {
            L2 = add_last_node(L2, temp->data); 
        }
        free(temp->data); 
        free(temp);       
    }
    return L2;
}


int main(){
    list L1 = NULL, L2 = NULL;
    //int creteria = 0;

    //printf(MAGENTA"Change criteria for the creteria to sort L1:\nlength - 0, althabetcal - 1: \n"RESET);
    //scanf("%d ", &creteria);
    
    printf(MAGENTA"Enter the words to create L1 (for the end:' '+Enter):\n"RESET);
    char * buf;
    while (1){
            buf = getString();
        if (buf[0] == '\0') {
            free(buf);
            break;
        }
        insert_node_sorted(&L1, buf, sort_criteria[0]);
        free(buf); 

	}
    
    printf(MAGENTA"Enter the words to create L2 (for the end:' '+Enter):\n"RESET);
    while (1){
		buf = getString();
		if (buf[0] == '\0') {
            free(buf);
            break;
        }
        insert_node_sorted(&L2, buf, sort_criteria[0]);
        free(buf); 
	}

    
    printf(GREEN "Printing L1 in length order:\n"RESET);    
    print_list_forward(L1);
    printf(GREEN "Printing L2 in althabetical order:\n"RESET); 
    print_list_forward(L2);
    
    L1 = merge_lists(L1, L2);
    
    printf(GREEN "Printing merged list:\n"RESET); 
    print_list_forward(L1);
    
    if (L1)
        delete_list(L1);
    return 0;
}