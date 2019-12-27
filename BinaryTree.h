#ifndef _BINARY_TREE_H_
#define _BINARY_TREE_H_

#include <iostream>

struct BinaryTreeNode
{
	int value;
	BinaryTreeNode *left;
	BinaryTreeNode *right;

	BinaryTreeNode()
	{
		value = 0;
		left = NULL;
		right = NULL;
	}
};

class BinaryTree
{
public:
	// 前序遍历
	static void PreorderTraversal(BinaryTreeNode *node);
	// 前序遍历非递归
	static void PreorderTraversal1(BinaryTreeNode *node);
	// 中序遍历
	static void InorderTraversal(BinaryTreeNode *node);
	// 中序遍历非递归
	static void InorderTraversal1(BinaryTreeNode *node);
	// 后序遍历
	static void PostorderTraversal(BinaryTreeNode *node);
	// 后序遍历非递归
	static void PostorderTraversal1(BinaryTreeNode *node);
	// 广度优先遍历
	static void BreadthFirstTraversal(BinaryTreeNode *node);

public:
	BinaryTree();

	bool InsertNode(int value);

	// 前序遍历
	void PreorderTraversal();
	// 前序遍历非递归
	void PreorderTraversal1();
	// 中序遍历
	void InorderTraversal();
	// 中序遍历非递归
	void InorderTraversal1();
	// 后序遍历
	void PostorderTraversal();
	// 后序遍历非递归
	void PostorderTraversal1();
	// 广度优先遍历
	void BreadthFirstTraversal();

private:
	bool insertNode(BinaryTreeNode *node, int value);

	BinaryTreeNode *root;
};

#endif


