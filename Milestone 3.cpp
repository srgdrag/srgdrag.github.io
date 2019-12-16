/*
 * Artifact2.cpp
 *
 *  Created on: Nov 24, 2019
 *      Author: Josh
 */

#include <iostream>
#include <time.h>
#include <algorithm>
#include <string>
#include <climits>

#include "CSVparser.hpp"

using namespace std;

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

// forward declarations
double strToDouble(string str, char ch);

// define a structure to hold bid information
struct Bid {
    string bidId; // unique identifier
    string title;
    string fund;
    double amount;
    Bid() {
        amount = 0.0;
    }
};

struct Node {
	Bid bid;
	struct Node *left;
	struct Node *right;
};

//============================================================================
// Binary Search Tree class definition
//============================================================================

/**
 * Define a class containing data members and methods to
 * implement a binary search tree
 */
class BinarySearchTree {

private:
    Node* root;

    void addNode(Node* node, Bid bid);
    void inOrder(Node* node);
    Node* removeNode(Node* node, string bidId);

public:
    BinarySearchTree();
    virtual ~BinarySearchTree();
    void InOrder();
    void Insert(Bid bid);
    void Remove(string bidId);
    Bid Search(string bidId);
};

/**
 * Default constructor
 */
BinarySearchTree::BinarySearchTree() {
    // initialize housekeeping variables
	this->root = new Node;
}

/**
 * Destructor
 */
BinarySearchTree::~BinarySearchTree() {
    // recurse from root deleting every node
}

/**
 * Traverse the tree in order
 */
void BinarySearchTree::InOrder() {
}
/**
 * Insert a bid
 */
void BinarySearchTree::Insert(Bid bid) {
	struct Node* temp = new Node; 	//temporary nodes to store information
	struct Node* curr = new Node;
	this->addNode(temp,bid);

	if (this->root == 0){    		// checks if tree is empty
		this->root = temp;
		temp->left = 0;
		temp->right = 0;
	}
	else {							//searches through tree and inserts node in the correct spot
		curr = this->root;
		while (curr != 0){
			if (temp->bid.bidId < curr->bid.bidId){
				if (curr->left == 0){
					curr->left = temp;
					curr = 0;
				}
				else {
					curr = curr->left;
				}
			}
			else {
				if (curr->right == 0){
					curr->right = temp;
					curr = 0;
				}
				else {
					curr = curr->right;
				}
			}
		}
		temp->left = 0;
		temp->right = 0;
	}
}

/**
 * Remove a bid
 */
void BinarySearchTree::Remove(string bidId) {
	struct Node* curr = new Node;	//temporary nodes to store information
	struct Node* par = new Node;
	struct Node* suc = new Node;

	par = 0;
	curr = this->root;
	while (curr != 0){				//searches for the node to be removed
		if (curr->bid.bidId == bidId){
			if (!curr->left && !curr->right){ 	//removes root node if its the only one left
				if (!par){
					this->root = 0;
				}
				else if (par->left == curr){
					par->left = 0;
				}
				else {
					par->right =0;
				}
			}
			else if (curr->left && !curr->right){	//only left side exists
				if (!par){
					this->root = curr->left;
				}
				else if (par->left == curr){
					par->left = curr->left;
				}
				else {
					par->right = curr->left;
				}
			}
			else if (!curr->left && curr->right){	//only right side exists
				if (!par){
					this->root = curr->right;
				}
				else if (par->left == curr){
					par->left = curr->right;
				}
				else {
					par->right = curr->right;
				}
			}
			else {									//both sides exist
				suc = curr->right;
				while (suc->left != 0){
					suc = suc->left;
				}
				curr = suc;
				curr = curr->right;
				bidId = suc->bid.bidId;
			}
			return;
		}
		else if (curr->bid.bidId < bidId){
			par = curr;
			curr = curr->right;
		}
		else {
			par = curr;
			curr = curr->left;
		}
	}
	return;
}

/**
 * Search for a bid
 */
Bid BinarySearchTree::Search(string bidId) {
	struct Node* curr = new Node;		//temporary node to store current spot
	Bid bid;

	curr = this->root;
	while (curr != 0){
		if (bidId == curr->bid.bidId){
			bid = curr->bid;
			return bid;
		}
		else if (bidId < curr->bid.bidId){	//moves down left branch
			curr = curr->left;
		}
		else {
			curr = curr->right;				//moves down right branch
		}
	}
	return bid;
}

/**
 * Add a bid to some node (recursive)
 *
 * @param node Current node in tree
 * @param bid Bid to be added
 */
void BinarySearchTree::addNode(Node* node, Bid bid) {
	node->bid = bid;
	node->left = 0;
	node->right = 0;
}
void BinarySearchTree::inOrder(Node* node) {
}

//============================================================================
// Linked-List class definition
//============================================================================

/**
 * Define a class containing data members and methods to
 * implement a linked-list.
 */
class LinkedList {

private:
    //Internal structure for list entries
	LinkedList* headPtr;
	LinkedList* tailPtr;
	LinkedList* nextNodeptr;
	Bid* bidVal;

public:
    LinkedList();
    virtual ~LinkedList();
    void Append(Bid bid);
    void Prepend(Bid bid);
    void PrintList();
    void Remove(string bidId);
    Bid Search(string bidId);
    int Size();
};

/**
 * Default constructor
 */
LinkedList::LinkedList() {
	this->headPtr = 0;
	this->tailPtr = 0;
	this->nextNodeptr = 0;
	this->bidVal = 0;
	return;
}

/**
 * Destructor
 */
LinkedList::~LinkedList() {
}

/**
 * Append a new bid to the end of the list
 */
void LinkedList::Append(Bid bid) {
 	bidVal = bid;
	if (this->headPtr == 0){
		this->headPtr = bidVal;
		this->tailPtr = bidVal;
	}
	else{
		this->tailPtr->nextNodeptr = bidVal;
		this->tailPtr = bidVal;
	}
}

/**
 * Prepend a new bid to the start of the list
 */
void LinkedList::Prepend(Bid bid) {
 	bidVal = bid;
	if (this->headPtr == 0) {
	      this->headPtr = bidVal;
	      this->tailPtr = bidVal;
	   }
	   else {
	      this->tailPtr ->nextNodeptr = this->headPtr;
	      this->headPtr = bidVal;
	   }
}

/**
 * Simple output of all bids in the list
 */
void LinkedList::PrintList() {
 	while (this->nextNodeptr != 0){
		cout << this->bidVal <<endl;
	}
}

/**
 * Remove a specified bid
 *
 * @param bidId The bid id to remove from the list
 */
void LinkedList::Remove(string bidId) {
	LinkedList* tgtBid = this->headPtr;
	LinkedList* tempPtr = 0;
	while(this->bidVal.bidId != bidId && tgtBid !=0){
		tgtBid = this->nextNodeptr;
	}
	if (tgtBid != 0){
		tempPtr = tgtBid->nextNodeptr->nextNodeptr;
		tgtBid->nextNodeptr = tempPtr;
		if (tempPtr == 0){
			this->tailPtr = tgtBid;
		}
	}
}

/**
 * Search for the specified bidId
 *
 * @param bidId The bid id to search for
 */
Bid LinkedList::Search(string bidId) {
 	LinkedList* tgtBid = this->headPtr;
	Bid foundBid= new Bid;
	while (tgtBid != 0){
		if(tgtBid->bidVal.bidId == bidId){
			foundBid = &tgtBid;
			return foundBid;
		}
		tgtBid = tgtBid ->nextNodeptr;
	}
	return foundBid;
}

/**
 * Returns the current size (number of elements) in the list
 */
int LinkedList::Size() {
    return size;
}

//============================================================================
// Hash Table class definition
//============================================================================

/**
 * Define a class containing data members and methods to
 * implement a hash table with chaining.
 */
class HashTable {

private:
  	vector<Bid> bid;
	list<Bid> *table;

    unsigned int hash(int key);

public:
    HashTable();
    virtual ~HashTable();
    void Insert(Bid bid);
    void PrintAll();
    void Remove(string bidId);
    Bid Search(string bidId);
};

/**
 * Default constructor
 */
HashTable::HashTable() {
	vector<Bid> bid;
	table = new list<Bid>[DEFAULT_SIZE];
}

/**
 * Destructor
 */
HashTable::~HashTable() {
	while (!bid.empty()){
		bid.pop_back();
	}
}

/**
 * Calculate the hash value of a given key.
 * Note that key is specifically defined as
 * unsigned int to prevent undefined results
 * of a negative list index.
 *
 * @param key The key to hash
 * @return The calculated hash
 */
unsigned int HashTable::hash(int key) {
 	return key % DEFAULT_SIZE;
}

/**
 * Insert a bid
 *
 * @param bid The bid to insert
 */
void HashTable::Insert(Bid bid) {
 	int id = 0;
	id = atoi (bid.bidId.c_str());
	int index = hash(id);
	table[index].push_back(bid);
}

/**
 * Print all bids
 */
void HashTable::PrintAll() {
  	for (int i = 0; i < DEFAULT_SIZE; i++) {
		cout << "Key " << i << ": ";
		for (Bid x : table[i]){
			cout << x.bidId << ": " << x.title << " | " << x.amount << " | "
		            << x.fund << endl;
		}
	}
}

/**
 * Remove a bid
 *
 * @param bidId The bid id to search for
 */
void HashTable::Remove(string bidId) {
 	int id = 0;
	id = atoi (bidId.c_str());
	int index = hash(id);
	list<int> :: iterator i;
	for ( i = table[index].begin(); i != table[index].end(); i++){
		if (*i == bidId)
	    	break;
	}
	if (i != table[index].end()){
		table[index].erase(i);
	}
}

/**
 * Search for the specified bidId
 *
 * @param bidId The bid id to search for
 */
Bid HashTable::Search(string bidId) {
    Bid bid;

    int id = 0;
    list<int> :: iterator i;
    id = atoi (bidId.c_str());
    int index = hash(id);
    for ( i = table[index].begin(); i != table[index].end(); i++){
    	if (*i == bidId)
    		bid = table[i];
    		break;
    }


    return bid;
}

//============================================================================
// Static methods used for testing
//============================================================================

/**
 * Display the bid information to the console (std::out)
 *
 * @param bid struct containing the bid info
 */
void displayBid(Bid bid) {
    cout << bid.bidId << ": " << bid.title << " | " << bid.amount << " | "
            << bid.fund << endl;
    return;
}

/**
 * Load a CSV file containing bids into a container
 *
 * @param csvPath the path to the CSV file to load
 * @return a container holding all the bids read
 */
void loadBids(string csvPath, BinarySearchTree* bst) {
    cout << "Loading CSV file " << csvPath << endl;

    // initialize the CSV Parser using the given path
    csv::Parser file = csv::Parser(csvPath);

    // read and display header row - optional
    vector<string> header = file.getHeader();
    for (auto const& c : header) {
        cout << c << " | ";
    }
    cout << "" << endl;

    try {
        // loop to read rows of a CSV file
        for (unsigned int i = 0; i < file.rowCount(); i++) {

            // Create a data structure and add to the collection of bids
            Bid bid;
            bid.bidId = file[i][1];
            bid.title = file[i][0];
            bid.fund = file[i][8];
            bid.amount = strToDouble(file[i][4], '$');

            //cout << "Item: " << bid.title << ", Fund: " << bid.fund << ", Amount: " << bid.amount << endl;

            // push this bid to the end
            bst->Insert(bid);
        }
    } catch (csv::Error &e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * Simple C function to convert a string to a double
 * after stripping out unwanted char
 *
 * credit: http://stackoverflow.com/a/24875936
 *
 * @param ch The character to strip out
 */
double strToDouble(string str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
    return atof(str.c_str());
}

/**
 * The one and only main() method
 */
int main(int argc, char* argv[]) {

    // process command line arguments
    string csvPath, bidKey;
    switch (argc) {
    case 2:
        csvPath = argv[1];
        bidKey = "98109";
        break;
    case 3:
        csvPath = argv[1];
        bidKey = argv[2];
        break;
    default:
        csvPath = "eBid_Monthly_Sales_Dec_2016.csv";
        bidKey = "98109";
    }

    // Define a timer variable
    clock_t ticks;

    // Define the various tables to hold all bids
    BinarySearchTree* bst;
    HashTable* bidTable;
    LinkedList bidList;

    Bid bid;

    int choice = 0;
    int sort = 0;

    cout << "Choose sort method" << endl;
    cout << "1. Binary search tree" << endl;
    cout << "2. Hash Table" << endl;
    cout << "3. Linked list" << endl;
    cin >> sort;

    if (!(sort >= 1 && sort <= 3))
    {
    	cout << "No sort method selected. Exiting program" << endl;
    	return 0;
    }

    while (choice != 9) {
        cout << "Menu:" << endl;
        cout << "  1. Load Bids" << endl;
        cout << "  2. Display All Bids" << endl;
        cout << "  3. Find Bid" << endl;
        cout << "  4. Remove Bid" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {

        case 1:
        	switch (sort)
        	{
        	case 1:
        		bst = new BinarySearchTree();

        		// Initialize a timer variable before loading bids
        		ticks = clock();

        		// Complete the method call to load the bids
        		loadBids(csvPath, bst);
        		break;

        	case 2:
        		bidTable = new HashTable();

        		// Initialize a timer variable before loading bids
        		ticks = clock();

        		// Complete the method call to load the bids
        		loadBids(csvPath, bidTable);
        		break;

        	case 3:
        		//initialize a timer variable before loading bids
        		ticks = clock();

        		// Complete the method call to load the bids
        		loadBids(csvPath, &bidList);
        		break;

        	}

            // Calculate elapsed time and display result
            ticks = clock() - ticks; // current clock ticks minus starting clock ticks
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
            break;

        case 2:
        	if (sort == 1)
        		bst->InOrder();
        	else if (sort == 2)
        		bidTable->PrintAll();
        	else if (sort == 3)
        		bidList.PrintList();
            break;

        case 3:
            ticks = clock();

            switch (sort)
            {
            case 1:
            	bid = bst->Search(bidKey);

            	ticks = clock() - ticks; // current clock ticks minus starting clock ticks

            	if (!bid.bidId.empty()) {
            		displayBid(bid);
            	} else {
            		cout << "Bid Id " << bidKey << " not found." << endl;
            	}
            	break;

            case 2:
            	bid = bidTable->Search(bidKey);

            	ticks = clock() - ticks; // current clock ticks minus starting clock ticks

            	if (!bid.bidId.empty())             	{
            		displayBid(bid);
            	} else {
            		cout << "Bid Id " << bidKey << " not found." << endl;
            	}
            	break;

            case 3:
            	bid = bidList.Search(bidKey);

            	ticks = clock() - ticks; // current clock ticks minus starting clock ticks

            	if (!bid.bidId.empty())             	{
            		displayBid(bid);
            	} else {
            		cout << "Bid Id " << bidKey << " not found." << endl;
            	}
            	break;
            }



            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;

            break;

        case 4:
        	if (sort == 1)
        		bst->Remove(bidKey);
        	else if(sort == 2)
        		bidTable->Remove(bidKey);
        	else if (sort == 3)
        		bidList.Remove(bidKey);
            break;
        }
    }

    cout << "Good bye." << endl;

	return 0;
}



