#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <unordered_map>
#include <vector>
#include "node.hpp"

/*		Archive file structure
 
	==========	HEADER	==========	#
	#								#
	unsigned size of archive		#
	unsigned header size			#
	unsigned number of vertecies	#
	tree nodes stored as:			#
	root -> left -> right			#
	(character -> frequency)		#
	#								#
	==============================	#

	======== FOR EACH FILE ========	#
	#								#
	====== INDIVIDUAL HEADER ======	#
	#								#
	unsigned size of original file	#
	unsigned size of header			#
	unsigned length of file name	#
	file name						#
	unsigned length of path			#
	file path						#
	#								#
	======= COMPRESSED DATA =======	#
	written byte by byte			#
	===============================	#
*/

namespace fs = std::filesystem;
using std::string, std::cout, std::endl, std::vector, std::ifstream, std::ofstream;

void compress(string& req_path);
void decompress(string& req_path);