#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct Node {
    char character;     
    int count;            
    struct Node* next;      
} Node;

Node* createNode(char character);
void insertOrUpdate(Node** head, char character);
void printList(Node* head);
void freeList(Node* head);

int main() {
    FILE* file = fopen("hw0.c","r"); 
    if (!file) {
        printf("cant open the file\n");
        return 1;
    }

    Node* head = NULL;
    char ch;


    while ((ch = fgetc(file)) != EOF) {
        insertOrUpdate(&head, ch);
    }

    fclose(file);

    printList(head);
    freeList(head);

    return 0;
}


Node* createNode(char character) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->character = character;
    newNode->count = 1;
    newNode->next = NULL;
    return newNode;
}
void insertOrUpdate(Node** head, char character) {
	if (character == '\n') {
        return;
    }
	if (character == '	') {
	        return;
	    }
    if (character == ' ') {
        return;
    }

    Node* current = *head;
    Node* previous = NULL;


    while (current != NULL) {
        if (current->character == character) {
            current->count++;
            return;
        }
        previous = current;
        current = current->next;
    }


    Node* newNode = createNode(character);
    if (previous == NULL) {
      
        newNode->next = *head;
        *head = newNode;
    } else {
      
        previous->next = newNode;
    }
}


void printList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        printf("%c : %d\n", current->character, current->count); 
        current = current->next;
    }
}


void freeList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
}
