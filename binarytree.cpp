#define BOOST_TEST_MODULE set_testing
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <boost/test/unit_test.hpp>

void error(const char* p) {
	printf("%s\n",p);
	abort();
}

class Node {
	friend class Set;
	friend class Iterator;

	int key;
	Node* right;
	Node* left;
	Node* parent;
public:
	static Node* leftmost;
	static Node* rightmost;
	static int nleft;
	static int nright;

	Node() : key(0), right(NULL), left(NULL), parent(NULL) {}
	Node* clone() const;
	Node* cloneRoot() const {
		nleft = nright = 0;
		leftmost = rightmost = NULL;
		return clone();
	}
	~Node() {
		if (left) delete left;
		if (right) delete right;
	}
};

Node* Node::leftmost = NULL;
Node* Node::rightmost = NULL;
int Node::nleft = 0;
int Node::nright = 0;

Node* Node::clone() const {
	Node* n = new Node;

	if (nright == 0) leftmost = n;
	if (nleft == 0)  rightmost = n;

	if (left) {
		nleft++;
		n->left = left->clone();
		n->left->parent = n;
		nleft--;
	}
	if (right) {
		nright++;
		n->right = right->clone();
		n->right->parent = n;
		nright--;
	}
	n->key = key;
	return n;
}

class Iterator {
	friend class Set;
	Node* node;
	void prev();
	void next();
	Node* maximum(Node*);
	Node* minimum(Node*);
public:
	Iterator() : node(NULL) {}
	Iterator(Iterator const& i) : node(i.node) {}
	explicit Iterator(Node* n): node(n) {}

	Iterator& operator=(Iterator const& i) { node = i.node; return *this; }

	Iterator& operator++() { next(); return *this; }
	Iterator operator++(int) { Iterator i=*this; next(); return i; }
	Iterator& operator--() { prev(); return *this; }
	Iterator operator--(int) { Iterator i=*this; prev(); return i; }

	int& operator*() const {if (node) return node->key; else error("* on NULL"); }

	bool operator==(Iterator const & i1) const {return (node == i1.node); }
	bool operator!=(Iterator const & i1) const {return !(*this == i1); }
};

Node* Iterator::maximum(Node* x) {
	return (x->right == NULL) ? x : maximum(x->right);
}

Node* Iterator::minimum(Node* x) {
	return (x->left == NULL) ? x : minimum(x->left);
}

void Iterator::prev() {
	if (node == NULL) {
		error("NULL prev");
		return;
	}
	if (node == reinterpret_cast<Node*>(reinterpret_cast<char*>(node) + 1)) {
		error("end prev");
		return;
	}
	if (node->left != NULL)
		node = maximum(node->left);
	else {
		Node* n = node;
		while (n->parent != NULL && n == n->parent->left)  
			n = n->parent;
		node = n->parent;
		if (node == NULL) error("begin prev");
	}
}

void Iterator::next() {
	if (node == NULL) {
		error("NULL next");
		return;
	}
	if (node == reinterpret_cast<Node*>(reinterpret_cast<char*>(node) + 1)) {
		error("end next");
		return;
	}
	if (node->right != NULL) 
		node = minimum(node->right);
	else {
		Node* n = node;
	    while (n->parent != NULL && n == n->parent->right)
			n = n->parent;
		if (n->parent == NULL) 
			node = reinterpret_cast<Node*>(reinterpret_cast<char*>(node) + 1);
		else 
			node = n->parent;
	}
}

class Set {
	Node* first;
	Node* last;
	Node* root;
public:
	Set() : root(NULL), first(NULL), last(NULL) {}
	Set(Set const& a): root(a.root->cloneRoot()) {
		first = Node::leftmost; 
		last = Node::rightmost;
	}
	Set& operator=(Set const& a) {
		if (root) delete root;
		root = a.root->cloneRoot();
		first = Node::leftmost;
		last = Node::rightmost;
	}

	void insert(int);
	Iterator find(int);
	void erase(Iterator);

	Iterator begin() const { return Iterator(first); }
	Iterator end() const { return Iterator(reinterpret_cast<Node*>(reinterpret_cast<char*>(last) + 1)); }

	~Set() {
		if (root) delete root;
	}
};

Iterator Set::find(int x) {
	Node* n = root;
	while (n != NULL && n->key != x) {
		if (x < n->key) 
			n = n->left;
		else n = n->right;
	}
	if (n == NULL) {
		return end();
	}
	else {
		return Iterator(n);
	}
}

void Set::insert(int x) {
	Node* n = root;
	Node* newN = new Node;
	if (n == NULL) {
		newN->key = x;
		root = newN;
		first = newN;
		last = newN;
		return;
	}
	if (find(x) != end()) {
		return;
	}
	for (;;) {
		if (x > n->key) {
			if (n->right != NULL) {
				n = n->right;
			}
			else {
				newN->parent = n;
				n->right = newN;
				newN->key = x;
				break;
			}
		}
		else {
			if (n->left != NULL) {
				n = n->left;
			}
			else {
				newN->parent = n;
				n->left = newN;
				newN->key = x;
				break;
			}
		}
	}
	if (x < first->key) first = newN;
	if (x > last->key) last = newN;
}

void Set::erase(Iterator i) {
	Node* n = i.node;
	if (last == n) {
		i--;
		last = i.node;
		i++;
	}
	else if (first == n) {
		i++;
		first = i.node;
		i--;
	}
	if (n == NULL) return;
	if (n->left != NULL && n->right != NULL) {
		i++;
		Node* y = i.node;
		n->key = y->key;
		n = y;
	}
	Node* child = (n->left == NULL ? n->right : n->left);
	if (child != NULL) child->parent = n->parent;
	if (n->parent == NULL) root = child;
	else if (n->parent->left == n) n->parent->left = child;
	else n->parent->right = child;
}


BOOST_AUTO_TEST_CASE(IterEqual) {
	Set bt;
    int a = 100;
	bt.insert(a);
	Iterator i = bt.find(a);
	Iterator j = i;
	BOOST_CHECK_EQUAL(i == j, true);
}

BOOST_AUTO_TEST_CASE(PlusPlusIter) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i = bt.find(a);
	++i;
	BOOST_CHECK_EQUAL(*i, b);
}

BOOST_AUTO_TEST_CASE(IterPlusPlus) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i = bt.find(a);
	i++;
	BOOST_CHECK_EQUAL(*i, b);
}

BOOST_AUTO_TEST_CASE(MinusMinusIter) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i;
	if (a > b) i = bt.find(a);
	else i = bt.find(b);
	--i;
	if (a > b) BOOST_CHECK_EQUAL(*i, b);
	else BOOST_CHECK_EQUAL(*i, a);
}

BOOST_AUTO_TEST_CASE(IterMinusMinus) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i;
	if (a > b) i = bt.find(a);
	else i = bt.find(b);
	i--;
	if (a > b) BOOST_CHECK_EQUAL(*i, b);
	else BOOST_CHECK_EQUAL(*i, a);
}

BOOST_AUTO_TEST_CASE(IterKey) {
	Set bt;
	int a = rand() % 100;
	bt.insert(a);
	Iterator i = bt.find(a);
	BOOST_CHECK_EQUAL(*i == a, true);
}

BOOST_AUTO_TEST_CASE(IterEqualEqual) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i = bt.find(a);
	Iterator j = bt.find(a);
	BOOST_CHECK_EQUAL(i == j, true);
}

BOOST_AUTO_TEST_CASE(IterNoEqual) {
	Set bt;
	int a = rand() % 100;
	int b = rand() % 100;
	bt.insert(a);
	bt.insert(b);
	Iterator i = bt.find(a);
	Iterator j = bt.find(b);
	BOOST_CHECK_EQUAL(i != j, true);
}


BOOST_AUTO_TEST_CASE(SetEqual) {
	Set bt;
	for (int i = 0; i < 10; i++) bt.insert(rand() % 100);
	Set btCopy = bt;
	Iterator j = btCopy.begin();
	for (Iterator i = bt.begin(); i != bt.end(); i++, j++) {
		BOOST_CHECK_EQUAL(*i, *j);
	}
}

BOOST_AUTO_TEST_CASE(SetInsert) {
	Set bt;
	for (int i = 9; i >= 0; i--) bt.insert(i);
	int j = 0;
	for (Iterator i = bt.begin(); i != bt.end(); i++) {
		BOOST_CHECK_EQUAL(*i == j, true);
		j++;
	}
}

BOOST_AUTO_TEST_CASE(SetFind) {
	Set bt;
	for (int i = 10; i > 0; i--) bt.insert(i);
	for (int i = 1; i <= 10; i++) {
		int a = *bt.find(i);
		BOOST_CHECK_EQUAL(a, i);
	}
}

BOOST_AUTO_TEST_CASE(SetErase) {
	Set bt;
	for (int i = 10; i > 0; i--) bt.insert(i);
	for (int i = 2; i < 10; i++) {
		bt.erase(bt.begin());
		int a = *bt.begin();
		BOOST_CHECK_EQUAL(a, i);
	}
}

BOOST_AUTO_TEST_CASE(SetBegin) {
	Set bt;
	for (int i = 10; i > 0; i--) bt.insert(i);
	for (int i = 2; i < 10; i++) {
		bt.erase(bt.begin());
		int a = *bt.begin();
		BOOST_CHECK_EQUAL(a, i);
	}
}

BOOST_AUTO_TEST_CASE(SetEnd) {
	Set bt;
	for (int i = 0; i < 10; i++) bt.insert(i);
	int j = 0;
	for (Iterator i = bt.begin(); i != bt.end(); i++) {
		j++;
	}
	BOOST_CHECK_EQUAL(j, 10);
}
