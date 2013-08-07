
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct _node_s;
typedef struct _node_s node_t;

struct _node_s {
	int id;
	node_t *left;
	node_t *right;
};

int pre_traverse(node_t *p_node);
int mid_traverse(node_t *p_node);
int post_traverse(node_t *p_node);
int add_node(node_t *p_node, node_t *new_node);

void show(int data)
{
	fprintf(stdout, "%d\n", data);
}

int pre_traverse(node_t *p_node)
{
	if (!p_node) return -1;
	
	show(p_node->data);
	pre_traverse(p_node->left);
	pre_traverse(p_node->right);

	return 0;
}

int mid_traverse(node_t *p_node)
{
	if (!p_node) return -1;
	
	mid_traverse(p_node->left);
	show(p_node->data);
	mid_traverse(p_node->right);

	return 0;
}

int post_traverse(node_t *p_node)
{
	if (!p_node) return -1;
	
	post_traverse(p_node->left);
	post_traverse(p_node->right);
	show(p_node->data);

	return 0;
}

int main(int argc, char *argv[])
{

	return 0;
}
