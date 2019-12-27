#include "BinaryTree.h"
#include <iostream>
#include <stack>
#include <queue>

// ǰ�����
void BinaryTree::PreorderTraversal(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	std::cout << node->value << ' ';
	PreorderTraversal(node->left);
	PreorderTraversal(node->right);
}
// ǰ������ǵݹ�
void BinaryTree::PreorderTraversal1(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	std::stack<BinaryTreeNode *> s;
	s.push(node);
	while (!s.empty())
	{
		BinaryTreeNode *tmp = s.top();
		s.pop();
		std::cout << tmp->value << ' ';
		if (tmp->right)
			s.push(tmp->right);
		if (tmp->left)
			s.push(tmp->left);
	}
}
// �������
void BinaryTree::InorderTraversal(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	InorderTraversal(node->left);
	std::cout << node->value << ' ';
	InorderTraversal(node->right);
}
// ��������ǵݹ�
void BinaryTree::InorderTraversal1(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	std::stack<BinaryTreeNode *> s;
	BinaryTreeNode *tmp = node;
	while (tmp || !s.empty())
	{
		if (tmp)
		{
			s.push(tmp);
			tmp = tmp->left;
		}
		else
		{
			tmp = s.top();
			s.pop();
			std::cout << tmp->value << ' ';
			tmp = tmp->right;
		}
	}
}
// �������
void BinaryTree::PostorderTraversal(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	PostorderTraversal(node->left);
	PostorderTraversal(node->right);
	std::cout << node->value << ' ';
}
// ����ǵݹ����
void BinaryTree::PostorderTraversal1(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	std::stack<BinaryTreeNode *> s1, s2;
	s1.push(node);
	while (!s1.empty())
	{
		BinaryTreeNode *tmp = s1.top();
		s1.pop();
		s2.push(tmp);
		if (tmp->left)
			s1.push(tmp->left);
		if (tmp->right)
			s1.push(tmp->right);
	}

	while (!s2.empty())
	{
		std::cout << s2.top()->value << ' ';
		s2.pop();
	}
}
// ������ȱ���
void BinaryTree::BreadthFirstTraversal(BinaryTreeNode *node)
{
	if (node == NULL)
		return;

	std::queue<BinaryTreeNode*> q;
	q.push(node);
	while (!q.empty())
	{
		BinaryTreeNode *tmp = q.front();
		q.pop();
		std::cout << tmp->value << ' ';
		if (tmp->left)
			q.push(tmp->left);
		if (tmp->right)
			q.push(tmp->right);
	}
}

BinaryTree::BinaryTree()
{
	root = NULL;
}

bool BinaryTree::InsertNode(int value)
{
	if (root == NULL)
	{
		root = new BinaryTreeNode;
		root->value = value;
	}
	else
	{
		insertNode(root, value);
	}

	return true;
}

// ǰ�����
void BinaryTree::PreorderTraversal()
{
	PreorderTraversal(root);
}
// ǰ��ǵݹ����
void BinaryTree::PreorderTraversal1()
{
	PreorderTraversal1(root);
}
// �������
void BinaryTree::InorderTraversal()
{
	InorderTraversal(root);
}
// ����ǵݹ����
void BinaryTree::InorderTraversal1()
{
	InorderTraversal1(root);
}
// �������
void BinaryTree::PostorderTraversal()
{
	PostorderTraversal(root);
}
// ����ǵݹ����
void BinaryTree::PostorderTraversal1()
{
	PostorderTraversal1(root);
}
// ������ȱ���
void BinaryTree::BreadthFirstTraversal()
{
	BreadthFirstTraversal(root);
}

bool BinaryTree::insertNode(BinaryTreeNode *node, int value)
{
	if (value <= node->value)
	{
		if (node->left == NULL)
		{
			BinaryTreeNode *childNode = new BinaryTreeNode;
			childNode->value = value;
			node->left = childNode;
		}
		else
		{
			insertNode(node->left, value);
		}
	}
	else
	{
		if (node->right == NULL)
		{
			BinaryTreeNode *childNode = new BinaryTreeNode;
			childNode->value = value;
			node->right = childNode;
		}
		else
		{
			insertNode(node->right, value);
		}
	}

	return true;
}