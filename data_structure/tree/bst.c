/*********************************************
* Author: zhangwj
* Filename: bst.c
* Descript: Binary Search Tree (BST) sample
* Date: 2017-02-07
* Warning: 
**********************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct bst_node {
	int data;
	struct bst_node *right;
	struct bst_node *left;
}Node;

/* 创建新节点 */
struct bst_node * bst_create_node(int data)
{
	struct bst_node *tmp = NULL;
	tmp = (struct bst_node *)malloc(sizeof(struct bst_node));
	if (NULL == tmp) {
		printf("create new node failure\n");
		return NULL;
	}
	tmp->data = data;
	tmp->right = NULL;
	tmp->left = NULL;
	
	return tmp;
}

/* 插入 */
static void bst_insert(struct bst_node **node, int data)
{
	if (NULL == *node) {
		*node = bst_create_node(data);
	} 
	else {
		if ((*node)->data == data) {
			return;
		} 
		else if (data < (*node)->data) {
			bst_insert(&((*node)->left), data);
		}
		else if (data > (*node)->data) {
			bst_insert(&((*node)->right), data);
		} 
	}	
}

/* 先序遍历 */
static void bst_preorder(struct bst_node *node)
{
	if (NULL == node) {
		return;
	}

	printf("%d ",node->data);
	bst_preorder(node->left);
	bst_preorder(node->right);
	return;
}

/* 中序遍历 */
static void bst_inorder(struct bst_node *node)
{
	if (NULL == node) {
		return;
	}

	bst_inorder(node->left);
	printf("%d ",node->data);
	bst_inorder(node->right);
	return;

}

/* 后序遍历 */
static void bst_postorder(struct bst_node *node)
{
	if (NULL == node) {
		return;
	}

	bst_postorder(node->left);
	bst_postorder(node->right);
	printf("%d ",node->data);
	return;

}

/* 查询 */
static void search_value(struct bst_node *node, int value)
{
	struct bst_node *find = NULL;

	if (NULL == node) {
		return;
	}

	find = node;	
	
	if (value < find->data) {
		search_value(find->left, value);
	} 
	else if (value > find->data) {
		search_value(find->right, value);
	} else {
		printf("have found:	[address: %p vlaue: %d]\n", find, find->data);
	}
}

/*销毁BST*/
static void bst_destroy(struct bst_node *node)
{
	if (NULL == node) {
		return;
	}
	
	if (NULL != node->right) {
		bst_destroy(node->right);
	}
	if (NULL != node->left) {
		bst_destroy(node->left);
	}
	free(node);
}


int main()
{
	struct bst_node * root = NULL;
	bst_insert(&root, 38);
	bst_insert(&root, 26);
	bst_insert(&root, 62);
	bst_insert(&root, 94);
	bst_insert(&root, 35);
	bst_insert(&root, 50);
	bst_insert(&root, 28);
	bst_insert(&root, 55);

	printf("preorder:	");
	bst_preorder(root);
	printf("\ninorder:	");
	bst_inorder(root);
	printf("\npostorder:	");
	bst_postorder(root);
	printf("\n");
	search_value(root, 35);

	bst_destroy(root);
	return 0;
}
