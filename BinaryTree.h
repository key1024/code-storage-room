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
	// ǰ�����
	static void PreorderTraversal(BinaryTreeNode *node);
	// ǰ������ǵݹ�
	static void PreorderTraversal1(BinaryTreeNode *node);
	// �������
	static void InorderTraversal(BinaryTreeNode *node);
	// ��������ǵݹ�
	static void InorderTraversal1(BinaryTreeNode *node);
	// �������
	static void PostorderTraversal(BinaryTreeNode *node);
	// ��������ǵݹ�
	static void PostorderTraversal1(BinaryTreeNode *node);
	// ������ȱ���
	static void BreadthFirstTraversal(BinaryTreeNode *node);

public:
	BinaryTree();

	bool InsertNode(int value);

	// ǰ�����
	void PreorderTraversal();
	// ǰ������ǵݹ�
	void PreorderTraversal1();
	// �������
	void InorderTraversal();
	// ��������ǵݹ�
	void InorderTraversal1();
	// �������
	void PostorderTraversal();
	// ��������ǵݹ�
	void PostorderTraversal1();
	// ������ȱ���
	void BreadthFirstTraversal();

private:
	bool insertNode(BinaryTreeNode *node, int value);

	BinaryTreeNode *root;
};

#endif


