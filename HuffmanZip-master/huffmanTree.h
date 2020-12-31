//哈夫曼树定义
#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include <bits/stdc++.h>
using namespace std;


//创建哈夫曼树。
typedef struct HuffmanTreeNode{
    int data;
    int weight;
    HuffmanTreeNode *parent,*lchild,*rchild,*address;

    HuffmanTreeNode():address(this){}
    HuffmanTreeNode(int d,int w):data(d), weight(w), parent(nullptr), lchild(nullptr), rchild(nullptr), address(this){}

    //重载>运算符，用于优先队列比较
    bool operator >(const HuffmanTreeNode other) const
    {
        return weight > other.weight;
    }
}Node;

//生成哈夫曼树并生成其哈夫曼编码表。

class HuffmanTree
{
public:
    HuffmanTree(int weights[]);//权值数组
    map<int,string> getCodeTable()
    {
        return codeTable;
    }//获取对应的编码表
    Node *getRoot()
    {
        return root;
    }//获取根节点的指针

private:
    map<int, string> codeTable;
    Node *root;

    Node* mergeTree(Node *tree1, Node *tree2);//合并两个哈夫曼树
    void makeCodeTable(Node *root,string code);//生成哈夫曼编码表
};

#endif // HUFFMANTREE_H
