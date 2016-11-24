#ifndef __LIST_H_
#define __LIST_H_

#include <ioCC2530.h>
#include <ZComDef.h>

#define  list_true     1
#define  list_false    0

typedef struct node{
	uint16 short_dev;
        uint8  addr_dev;
	struct node* pNext;
}Node;

extern Node *head ;
void reverseNodeList(void);
void sort(void);
void destroyNodeList(void);
extern bool createNodelist(void);
extern uint16 get_NodeList(uint8 dev);
void setNodeList(Node * node);
bool addNode(Node * node);
extern bool deleteNode(uint8 index);

#endif