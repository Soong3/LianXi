


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

#define RED				1
#define BLACK 			2

rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x) {	//寻找当前节点的前驱节点
	while (x->left != T->nil) {
		x = x->left;
	}
	return x;
}

rbtree_node *rbtree_maxi(rbtree *T, rbtree_node *x) {	//寻找当前节点的后继节点
	while (x->right != T->nil) {
		x = x->right;
	}
	return x;
}

rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x) {
	rbtree_node *y = x->parent;

	if (x->right != T->nil) {//如果当前节点的右子树不为空，直接返回当前节点的后继节点
		return rbtree_mini(T, x->right);
	}

	while ((y != T->nil) && (x == y->right)) {
		x = y;
		y = y->parent;
	}
	return y;
}


void rbtree_left_rotate(rbtree *T, rbtree_node *x) {

	rbtree_node *y = x->right;  // x  --> y  ,  y --> x,   right --> left,  left --> right

	//修改node_r节点的left与node之间指向关系
	x->right = y->left; //1 1
	if (y->left != T->nil) { //1 2
		y->left->parent = x;
	}

	//修改node的parent与node_r之间的指向关系
	y->parent = x->parent; //1 3
	if (x->parent == T->nil) { //1 4
		T->root = y;
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}

	//修改node与node_r之间的指向关系
	y->left = x; //1 5
	x->parent = y; //1 6
}


void rbtree_right_rotate(rbtree *T, rbtree_node *y) {

	rbtree_node *x = y->left;

	//修改node_r节点的left与node之间指向关系
	y->left = x->right;
	if (x->right != T->nil) {
		x->right->parent = y;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	} else if (y == y->parent->right) {
		y->parent->right = x;
	} else {
		y->parent->left = x;
	}

	x->right = y;
	y->parent = x;
}

void rbtree_insert_fixup(rbtree *T, rbtree_node *z) {

	while (z->parent->color == RED) { //z ---> RED
		if (z->parent == z->parent->parent->left) {
			rbtree_node *y = z->parent->parent->right;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;

				z = z->parent->parent; //z --> RED
			} else {

				if (z == z->parent->right) {
					z = z->parent;
					rbtree_left_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_right_rotate(T, z->parent->parent);
			}
		}else {
			rbtree_node *y = z->parent->parent->left;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;

				z = z->parent->parent; //z --> RED
			} else {
				if (z == z->parent->left) {
					z = z->parent;
					rbtree_right_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_left_rotate(T, z->parent->parent);
			}
		}
		
	}

	T->root->color = BLACK;
}


void rbtree_insert(rbtree *T, rbtree_node *z) {

	rbtree_node *y = T->nil;
	rbtree_node *x = T->root;

	while (x != T->nil) {
		y = x;
#if ENABLE_KEY_CHAR
		if (strcmp(z->key, x->key) < 0) {
			x = x->left;
		} else if (strcmp(z->key, x->key) > 0) {
			x = x->right;
		} else { //Exist
			return ;
		}
#else
		if (z->key < x->key) {
			x = x->left;
		} else if (z->key > x->key) {
			x = x->right;
		} else { //Exist
			return ;
		}
#endif
	}

	z->parent = y;
	if (y == T->nil) {
		T->root = z;
#if ENABLE_KEY_CHAR
	} else if (strcmp(z->key, y->key) < 0) {
#else
	} else if (z->key < y->key) {
#endif
		y->left = z;
	} else {
		y->right = z;
	}

	z->left = T->nil;
	z->right = T->nil;
	z->color = RED;

	rbtree_insert_fixup(T, z);
}

void rbtree_delete_fixup(rbtree *T, rbtree_node *x) {

	while ((x != T->root) && (x->color == BLACK)) {
		if (x == x->parent->left) {

			rbtree_node *w= x->parent->right;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;

				rbtree_left_rotate(T, x->parent);
				w = x->parent->right;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					rbtree_right_rotate(T, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rbtree_left_rotate(T, x->parent);

				x = T->root;
			}

		} else {

			rbtree_node *w = x->parent->left;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rbtree_right_rotate(T, x->parent);
				w = x->parent->left;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rbtree_left_rotate(T, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rbtree_right_rotate(T, x->parent);

				x = T->root;
			}

		}
	}

	x->color = BLACK;
}

rbtree_node *rbtree_delete(rbtree *T, rbtree_node *z) {

	rbtree_node *y = T->nil;
	rbtree_node *x = T->nil;

	if ((z->left == T->nil) || (z->right == T->nil)) {
		y = z;
	} else {
		y = rbtree_successor(T, z);
	}

	if (y->left != T->nil) {
		x = y->left;
	} else if (y->right != T->nil) {
		x = y->right;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	} else if (y == y->parent->left) {
		y->parent->left = x;
	} else {
		y->parent->right = x;
	}

	if (y != z) {
#if ENABLE_KEY_CHAR
		void *temp = z->key;
		z->key = y->key;
		y->key = temp;

		temp = z->value;
		z->value = y->value;
		y->value = temp;
#else
		z->key = y->key;
		z->value = y->value;
#endif
	}

	if (y->color == BLACK) {
		rbtree_delete_fixup(T, x);
	}

	return y;
}

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key) {

	rbtree_node *node = T->root;
	while (node != T->nil) {
#if	ENABLE_KEY_CHAR
		if (strcmp(key, node->key) < 0) {
			node = node->left;
		} else if (strcmp(key, node->key) > 0) {
			node = node->right;
		} else {
			return node;
		}
#else
		if (key < node->key) {
			node = node->left;
		} else if (key > node->key) {
			node = node->right;
		} else {
			return node;
		}	
#endif
	}
	return T->nil;
}


void rbtree_traversal(rbtree *T, rbtree_node *node) {
	if (node != T->nil) {
		rbtree_traversal(T, node->left);
		printf("key:%s, color:%d\n", node->key, node->color);
		rbtree_traversal(T, node->right);
	}
}

//接口实现部分
rbtree global_tree ;
int kvs_rbtree_create(rbtree *tree){
	if (tree == NULL) {
		return 1;
	}
	tree->nil = (rbtree_node *)kvs_malloc(sizeof(rbtree_node));//nil节点
	tree->nil->color = BLACK;//nil节点为黑色
	tree->root = tree->nil;//根节点为nil
	return 0;
}

void kvs_rbtree_destroy(rbtree *tree){
    if (tree == NULL) {
        return;
    }
	while (tree->root != tree->nil) 
	{
		rbtree_node *mini = rbtree_mini(tree,tree->root);
        rbtree_node *cur = rbtree_delete(tree,mini);
        kvs_free(cur->key);
        kvs_free(cur->value);
        kvs_free(cur);
	}
	kvs_free(tree->nil);
}

//1表示已存在，0表示插入成功
int kvs_rbtree_set(rbtree *tree, KEY_TYPE key, KEY_TYPE value){
	if(!tree || !key || !value)return-1;
    rbtree_node *cur = rbtree_search(tree,key);
    if(cur != tree->nil)return 1;
    cur = (rbtree_node*)kvs_malloc(sizeof(rbtree_node));
    if(cur == NULL)return -2;
 
    int length = strlen(key)+1;
    cur->key = kvs_malloc(length);
    if(cur->key == NULL)return -2;
    memset(cur->key,0,length);
    strcpy(cur->key,key);
 
    length = strlen(value)+1;
    cur->value = kvs_malloc(length);
    if(cur->value == NULL)return -2;
    memset(cur->value,0,length);
    strcpy(cur->value,value);
 
    rbtree_insert(tree,cur);
    return 0;
}

//NULL表示没有改节点
char *kvs_rbtree_get(rbtree *tree, KEY_TYPE key){
    if(!key || !tree) return NULL;
 
    rbtree_node *cur = rbtree_search(tree,key);
    if(cur == tree->nil) return NULL;
    
    return cur->value;
}

//大于0表示不存在，0表示删除成功
int kvs_rbtree_delete(rbtree *tree, KEY_TYPE key){
    if(!key || !tree) return -1;
    rbtree_node *cur = rbtree_search(tree,key);
    if(cur == tree->nil) return 1;
 
    rbtree_node *node = rbtree_delete(tree,cur);
    kvs_free(node->key);
    kvs_free(node->value);
    kvs_free(node);
    return 0;

}

//大于0表示不存在，0表示修改成功
int kvs_rbtree_modify(rbtree *tree, KEY_TYPE key, KEY_TYPE value){
    if(!tree || !key || !value) return-1;
 
    rbtree_node *cur = rbtree_search(tree,key);
    if(cur == tree->nil) return 1;
 
    kvs_free(cur->value);
    int length = strlen(value) + 1;
    cur->value =kvs_malloc(length);
    if(cur->value == NULL) return -2;
    memset(cur->value,0,length);
    strcpy(cur->value,value);
 
    return 0;
}
int kvs_rbtree_exists(rbtree *tree, KEY_TYPE key){
    if(!tree || !key) return -1;
 
    rbtree_node *node = rbtree_search(tree,key);
    if(node == tree->nil) return 1;
 
    return 0;
}

#if 0
int main() {

#if ENABLE_KEY_CHAR
	char* keyArray[10] = {"King", "Darren", "Mark", "Vico", "Nick", "qiuxiang", "youzi", "taozi", "123", "234"};
	char* valueArray[10] = {"1King", "2Darren", "3Mark", "4Vico", "5Nick", "6qiuxiang", "7youzi", "8taozi", "9123", "10234"};
 
	rbtree *T = (rbtree *)malloc(sizeof(rbtree));
	if (T == NULL) {
		printf("malloc failed\n");
		return -1;
	}
	
	T->nil = (rbtree_node*)malloc(sizeof(rbtree_node));
	T->nil->color = BLACK;
	T->root = T->nil;
 
	rbtree_node *node = T->nil;
	int i = 0;
	for (i = 0;i < 10;i ++) {
		node = (rbtree_node*)malloc(sizeof(rbtree_node));
		
		node->key = malloc(strlen(keyArray[i]) + 1);
		memset(node->key, 0, strlen(keyArray[i]) + 1);
		strcpy(node->key, keyArray[i]);
		
		node->value = malloc(strlen(valueArray[i]) + 1);
		memset(node->value, 0, strlen(valueArray[i]) + 1);
		strcpy(node->value, valueArray[i]);
		rbtree_insert(T, node);
	}
 
	rbtree_traversal(T, T->root);
	printf("----------------------------------------\n");
 
	for (i = 0;i < 10;i ++) {
 
		rbtree_node *node = rbtree_search(T, keyArray[i]);
		rbtree_node *cur = rbtree_delete(T, node);
		free(cur);
 
		rbtree_traversal(T, T->root);
		printf("----------------------------------------\n");
	}

#else
	int keyArray[20] = {24,25,13,35,23, 26,67,47,38,98, 20,19,17,49,12, 21,9,18,14,15};

	rbtree *T = (rbtree *)malloc(sizeof(rbtree));
	if (T == NULL) {
		printf("malloc failed\n");
		return -1;
	}
	
	T->nil = (rbtree_node*)malloc(sizeof(rbtree_node));
	T->nil->color = BLACK;
	T->root = T->nil;

	rbtree_node *node = T->nil;
	int i = 0;
	for (i = 0;i < 20;i ++) {
		node = (rbtree_node*)malloc(sizeof(rbtree_node));
		node->key = keyArray[i];
		node->value = NULL;

		rbtree_insert(T, node);
		
	}

	rbtree_traversal(T, T->root);
	printf("----------------------------------------\n");

	for (i = 0;i < 20;i ++) {

		rbtree_node *node = rbtree_search(T, keyArray[i]);
		rbtree_node *cur = rbtree_delete(T, node);
		free(cur);

		rbtree_traversal(T, T->root);
		printf("----------------------------------------\n");
	}
#endif	

	
}
#endif 



