// Starting date: 16/11/20
// Creator: Danver Zhao

// This program is like a simplified information retrieve system, where you 
// can search for words and get where it is from and the corresponding TfIdf
// value.

#include<stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "invertedIndex.h" 

#define MAX_WORD_LENGTH 20
#define MAX_FILE_SIZE 250


InvertedIndexBST createTree(InvertedIndexBST root, char *data,
	double tf_value, char *file_name);
InvertedIndexBST createTreeNode(char *data);
FileList createflnode(char *file_name, double tf_value);
void join_Tnode_flnode(InvertedIndexBST Tnode, FileList flnode);
int num_of_elm(InvertedIndexBST tree);
TfIdfList create_TfIdf(TfIdfList list_head, char *file_name, double tfIdfSum);
InvertedIndexBST search_word(InvertedIndexBST tree, char * searchWord);
TfIdfList merge_lists(TfIdfList final_list, TfIdfList unordered_list);
TfIdfList merge_files(TfIdfList final_list);
void print_helper(FILE *inverted_pt, InvertedIndexBST tree);

// Normalising words: change to all lower case, get rid of last character if 
// is it "," or "." or ";" or "?", returns the normalised word.
char *normaliseWord(char *str) {
	
	for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }

	int len = strlen(str) - 1;	
	
	if ((str[len] == ',') || (str[len] == '.') 
		|| (str[len] == ';') || (str[len] == '?')) {
		str[len] = '\0';
	}
		
    return str;
}

// Generates a binary search tree of normalised words, each word has a linked 
// list of which file the word is from and it's tf value returns the start of
// the bstree.
InvertedIndexBST generateInvertedIndex(char *collectionFliename){
	
	// Open file, gets the pointer
	FILE *collection_pointer = fopen(collectionFliename, "r");
	
	// Read in each word and creates InveretedIndex tree
	char file_name[30];
	char data[30];
	InvertedIndexBST root = NULL;
	// Opening the collection file and read the filenames
	while (fscanf(collection_pointer, "%s", file_name) != EOF) {
		// Opening the filename inside the collection and read data
		FILE *file_pointer = fopen(file_name, "r");
		double total_word_count = 0;

		// Opening the file 1st time to get total word count 
		while (fscanf(file_pointer, "%s", data) != EOF) {
			total_word_count++;
		}
		fseek(file_pointer, 0, SEEK_SET);

		// Opening the file 2nd time to get word frequency
		while (fscanf(file_pointer, "%s", data) != EOF) {
			// Insert the word into the tree
			double tf_value = (1 / total_word_count); 
			char* new_data = normaliseWord(data);
			root = createTree(root, new_data, tf_value, file_name);
		}
		
		// Close file
		fclose(file_pointer);
	}
	
	// Close collection file
	fclose(collection_pointer);

	return root; 
}

// Prints the InvertedIndex(bstree of normalised words)into InvertedIndex.txt, 
// one line per word in alphabetical order, list of file name is also 
// alphabetical ordered.
// format:
// word filename (tf value)
// word filename (tf value)
void printInvertedIndex(InvertedIndexBST tree){
	// Opens the collection of filenames 
	FILE *inverted_pt = fopen("invertedIndex.txt", "w");
	// Outsource the printing to a helper function called print_helper
	print_helper(inverted_pt, tree);
	// closes the collection of filenames
	fclose(inverted_pt);
}

// Given a word returns an ordered list where each node contains a filename
// and the corresponding tf-idf value. 
TfIdfList calculateTfIdf(InvertedIndexBST tree, char *searchWord, int D) {
	// Find file count for each word
	InvertedIndexBST s_word = search_word(tree, searchWord);
	FileList curr = s_word->fileList;
	double file_count = 0;
	while (curr != NULL) {
		file_count++;
		curr = curr->next;
	}
	// Creating the ordered list
	TfIdfList list_head = NULL;
	curr = s_word->fileList;
	//printf("Search word is %s\n", searchWord);
	while (curr != NULL) {
		//printf("File name is : %s\n", curr->filename);
		double TfIdf = log10(D / file_count) * curr->tf;
		list_head = create_TfIdf(list_head, curr->filename, TfIdf);
		curr = curr->next;
	}
	return list_head;
	
}

// The input is multiple words and everything else works like the function 
// calculateTfIdf.
TfIdfList retrieve(InvertedIndexBST tree, char *searchWords[], int D) {
	// Look for each word in the array
	TfIdfList final_list = NULL;
	int counter = 0;
	while (searchWords[counter] != NULL) {
		// Create unordered list with all term matching with searchWords
		TfIdfList unordered_list = calculateTfIdf(tree, searchWords[counter], D);
		// Merge all lists for each word into one list.
		final_list = merge_lists(final_list, unordered_list);
		counter++;
	}
	// From the one list pick out ones with the same filename and make a new
	// list.
	TfIdfList final_ordered_list = merge_files(final_list);
	return final_ordered_list;
}















//--------------------------------------------------------------------------
// Helper functions
// Creating bstree of normalised words.
InvertedIndexBST createTree(InvertedIndexBST root, char *data, 
	double tf_value, char *file_name) {

// Base case for inserting, returns the start of the tree.
	if (root == NULL) {
		InvertedIndexBST Tnode = createTreeNode(data);
		FileList flnode = createflnode(file_name, tf_value);
		join_Tnode_flnode(Tnode, flnode);
		return Tnode;
	}
	// 1st string < 2nd string 
	else if (strcmp(data, root->word) < 0) {
		InvertedIndexBST Tnode = createTreeNode(data);
		FileList flnode = createflnode(file_name, tf_value);
		join_Tnode_flnode(Tnode, flnode);
		root->left = createTree(root->left, data, tf_value, file_name);
	}
	// 1st string > 2nd string
	else if (strcmp(data, root->word) > 0) {
		InvertedIndexBST Tnode = createTreeNode(data);
		FileList flnode = createflnode(file_name, tf_value);
		join_Tnode_flnode(Tnode, flnode);
		root->right = createTree(root->right, data, tf_value, file_name);
	}
	// The same word // make another function (alphabetical order)
	else if (strcmp(data, root->word) == 0) {
		// The word is from the same file, just add the tf value
		FileList curr = root->fileList;
		while (curr != NULL) {
			if (strcmp(file_name, curr->filename) == 0) {
				curr->tf = curr->tf + tf_value;
				return root;
			}
			curr = curr->next;
		}
	
		// The word is from a different file
		FileList flnode = createflnode(file_name, tf_value);
		join_Tnode_flnode(root, flnode);
	
	}
	return root;
}

// Using normalised words to make nodes for the tree.
InvertedIndexBST createTreeNode(char *data) {
	InvertedIndexBST Tnode = malloc(sizeof(struct InvertedIndexNode));
	Tnode->word = malloc(MAX_WORD_LENGTH);
	strncpy(Tnode->word, data, MAX_WORD_LENGTH);
	Tnode->fileList = NULL;
	Tnode->right = NULL;
	Tnode->left = NULL;
	return Tnode;
}

// Making nodes for filenames and tf-value.
FileList createflnode(char *file_name, double tf_value) {
	FileList flnode = malloc(sizeof(struct FileListNode));
	flnode->filename = malloc(MAX_WORD_LENGTH);
	strncpy(flnode->filename, file_name, MAX_WORD_LENGTH);
	flnode->tf = tf_value;
	flnode->next = NULL;
	return flnode;
}

// Linking the file list nodes together in alphabetical order.
void join_Tnode_flnode(InvertedIndexBST Tnode, FileList flnode) {
	FileList curr = Tnode->fileList;
	FileList prev = NULL;
	// Check if it's first time 
	if (curr == NULL) {
		Tnode->fileList = flnode;
	}
	else {
		while (curr != NULL) {
			// File name is greater than curr name
			if (strcmp(flnode->filename, curr->filename) > 0) {
				prev = curr;
				curr = curr->next;
			}
			// File name is less than first flnode
			else if (strcmp(flnode->filename, curr->filename) < 0 
				&& prev == NULL) {
				flnode->next = Tnode->fileList;
				Tnode->fileList = flnode;
				break;
			}
			
			// File name is less than curr name
			else if (strcmp(flnode->filename, curr->filename) < 0) {
				prev->next = flnode;
				flnode->next = curr;
				break;
			}
		}
		if (curr == NULL) {
			prev->next = flnode;
		}
	}
}

// Returning an ordered list where each node contains a filename and the 
// corresponding tf-idf value for a given searchWord.
TfIdfList create_TfIdf(TfIdfList list_head, char *file_name, double tfIdfSum) {
	// Creating the node for TfIdf 
	TfIdfList TfIdf_node = malloc(sizeof(struct TfIdfNode));
	TfIdf_node->filename = malloc(MAX_WORD_LENGTH);
	strncpy(TfIdf_node->filename, file_name, MAX_WORD_LENGTH);
	TfIdf_node->tfIdfSum = tfIdfSum;
	TfIdf_node->next = NULL;
	
	// Creating the list, insertion
	if (list_head == NULL) {
		return TfIdf_node;
	}
	TfIdfList curr = list_head;
	TfIdfList prev = NULL;
	while (curr != NULL) {
		// If value is smaller than curr, keep looping, small value is 
		// at the back
		if (tfIdfSum < curr->tfIdfSum) {
			prev = curr;
			curr = curr->next;
		}
		// If the value is the biggest, List head changes
		else if (tfIdfSum > curr->tfIdfSum && prev == NULL) {
			TfIdf_node->next = list_head;	
			return TfIdf_node;
		}
		// Normal insertion
		else if (tfIdfSum > curr->tfIdfSum) {
			prev->next = TfIdf_node;
			TfIdf_node->next = curr;	
			return list_head;
		}
		// If same value, arrange in ascending file name
		else if (tfIdfSum == curr->tfIdfSum) {
			// loop through all the files with the same ftidf value
			while (tfIdfSum == curr->tfIdfSum) {
				// If this is the very first file
				if (strcmp(TfIdf_node->filename, curr->filename) < 0 
					&& prev == NULL) {
					TfIdf_node->next = curr;
					return TfIdf_node;
				}
				else if (strcmp(TfIdf_node->filename, curr->filename) < 0) {
					prev->next = TfIdf_node;
					TfIdf_node->next = curr;
					return list_head;
				}
				else if (strcmp(TfIdf_node->filename, curr->filename) > 0) {
					prev = curr;
					curr = curr->next;
				}
			}
			// When value is at the end of "same TfIdf value"
			prev->next = TfIdf_node;
			TfIdf_node->next = curr;
			return list_head;
		}
	}
	// When the	 value is at the very end of the list
	prev->next = TfIdf_node;
	return list_head;

}

// Returns the node which has the given word in bstree.
InvertedIndexBST search_word(InvertedIndexBST tree, char *searchWord) {
	if (strcmp(searchWord, tree->word) == 0) {
		return tree;
	}
	else if (strcmp(searchWord, tree->word) < 0) {
		return search_word(tree->left, searchWord);
	}
	else if (strcmp(searchWord, tree->word) > 0) {
		return search_word(tree->right, searchWord);
	}
	return NULL;
}

// Joining for lists head to tail.
TfIdfList merge_lists(TfIdfList final_list, TfIdfList unordered_list) {
	if (final_list == NULL) {
		return unordered_list;
	}
	// Join the two list head to tail
	else {
		TfIdfList curr = final_list;
		TfIdfList prev = NULL;
		while (curr != NULL) {
			prev = curr;
			curr = curr->next;
		}
		prev->next = unordered_list;
	}	
	return final_list;
}

// Making the final version of the ordered list where each node contains a 
// filename and the summation of tf-idf values of all the matching searchWords
// for that file.
TfIdfList merge_files(TfIdfList final_list) {
	// Look for all node with same file name
	TfIdfList fffinal_list = NULL;
	// While the unordered list in not empty
	while (final_list != NULL) {
		// Keep searching for same file name
		TfIdfList curr = final_list;
		TfIdfList prev = NULL;
		TfIdfList temp_prev = NULL;
		TfIdfList anchor = NULL;
		//TfIdfList search_node = final_list;
		char *f_name = final_list->filename;
		double TfIdf_sum = 0;
		while (curr != NULL) {
			// If same file name
			if (strcmp(f_name, curr->filename) == 0) {
				TfIdf_sum = TfIdf_sum + curr->tfIdfSum;
				temp_prev = prev;
				prev = curr;
				curr = curr->next;
				// Delete the node
				// If the node is the first node
				if (prev == final_list) {
					free(prev);
					final_list = curr;
				}
				else {
					free(prev);
					// If the are deleted nodes in a row e.g two deleted node 
					// between anchor and curr
					if (temp_prev == NULL && prev == NULL) {
						anchor->next = curr;
					}
					// Normal deletion
					else {
						temp_prev->next = curr;
						anchor = temp_prev;
					}
				}
			}
			// Going through the list
			else {
				prev = curr;
				curr = curr->next;
			}
		}
		// When one cycle is done, make the new list with only one file type
		fffinal_list = create_TfIdf(fffinal_list, f_name, TfIdf_sum);
	}
	return fffinal_list;
}

// Prints out the bstree into InvertedIndex.txt.
void print_helper(FILE *inverted_pt, InvertedIndexBST tree) {
	if (tree == NULL) {
		return;
	}
	
	// print left sub tree
	print_helper(inverted_pt, tree->left);

	// print word then all files containing it
	fprintf(inverted_pt, "%s ", tree->word);
	FileList curr = tree->fileList;
	while (curr != NULL) {
		fprintf(inverted_pt, "%s (%.6lf) ", curr->filename, curr->tf);
		curr = curr->next;
	}
	fprintf(inverted_pt, "\n");

	// print right sub tree
	print_helper(inverted_pt, tree->right);
}
