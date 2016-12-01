#include "OSAL.h"
#include "serial-comm.h"
#include "list.h"
//创建链表，头结点data=0;pNext=null

Node* head = NULL;
bool createNodelist(void)
{
	head = (Node*)osal_mem_alloc(sizeof(Node));
        //分配一个头结点
	if(NULL == head){
		return list_false;
		}
	else {
		head->short_dev =0;
                head->addr_dev =0;
		head->pNext = NULL;
		return list_true;
		}
}

//增加节点尾部添加
bool addNode(Node * node)
{
	if(NULL == head){
	  return list_false;
		}
	Node* p = head ->pNext;
	Node* q = head;
	while(NULL !=p){
		q = p;
		p = p->pNext;
		}
	q->pNext = node;
	node ->pNext = NULL;	
	return list_true;
}

//删除节点
bool deleteNode(uint8 index)
{
	if(NULL == head){
	return list_false;
		}
	Node* perior = head;
	Node* q = head->pNext;
	while((q!=NULL)&&(q->addr_dev != index))
		{
		perior = q;
		q = q->pNext;
		}
	if(q->addr_dev == index)
		{
		if(q->pNext == NULL)
			{
			perior->pNext = NULL;
			return list_true;
			}
		else
			{
			perior->pNext = q->pNext;
			return list_true;
			}
		}
        return list_false;
}

//销毁
void destroyNodeList(void)
{
	if(NULL == head){
		return;
		}
	if(NULL == head->pNext){
		osal_mem_free(head);
		head =NULL;
		return;
		}
	Node* p = head->pNext;
	while(NULL != p){
		Node* tmp =p;
		p = p->pNext;
		osal_mem_free(tmp);
		}
	osal_mem_free(head);
	head = NULL;
}


bool setNodeList(uint8 *buf, uint16 len)
{
    uint16 dev_shortaddr;
    uint8 dev_addr;
    uint8 flag=1;
    uint8 data_buf[3];
    osal_memcpy(data_buf,buf,len);
    dev_shortaddr=(uint16)(data_buf[0])<<8 | (uint16)data_buf[1];
    dev_addr=data_buf[2];
    if(NULL == head)
    {
      return list_false;
    }
    Node* p = head->pNext;
    while(NULL !=p)
    {
      if(p->addr_dev==dev_addr)
      {
        flag=0;
        p->short_dev=dev_shortaddr;//if in the list,cover the short_de
      }
      p = p->pNext;
    }
    if(1==flag)
    {
       Node *node = (Node*)osal_mem_alloc(sizeof(Node));
       node->short_dev=dev_shortaddr;
       node->addr_dev=dev_addr;
       node->pNext=NULL;
       addNode(node);
    }
    return list_true;
}

uint16 get_NodeList(uint8 dev)
{
   Node* p = head->pNext;
  while(NULL != p)
  {
    if(p->addr_dev==dev)
    {
     return p->short_dev;
    }
    else
    p=p->pNext;
  }
  return 0;
}
