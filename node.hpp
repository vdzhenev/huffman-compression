#pragma once
typedef unsigned char uchar;
const uchar SENTINEL = '\0';
const unsigned FLAG = 0;


//Node for Huffman tree
struct Node
{
	uchar c;			//unsigned char c = 1 byte, stores the byte
	unsigned freq;		//unsigned freq - the frequency of c (how many times we find c in the file)
	Node* left;			//left and right children
	Node* right;

	Node(uchar c = SENTINEL, unsigned freq = 1, Node* left = nullptr, Node* right = nullptr) :c(c), freq(freq),left(left),right(right) {};
};

//Struct, used to compare Nodes in priority queue (ascending order based on frequency)
struct CompareNode
{
	bool operator()(Node const* n1, Node const* n2) { return n1->freq > n2->freq; }
};