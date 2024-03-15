#include "algorithm.hpp"

typedef std::priority_queue<Node*, vector<Node*>, CompareNode> NodeQ;
typedef std::unordered_map<uchar, Node*> CtoN;
typedef std::unordered_map<uchar, string> CtoCode;

//Creates tree by given priority queue
void createHuffTree(NodeQ& Q)
{
	//Combines the first 2 Nodes, storing the least frequently found byte into a new node with combined frequency and sentinel as a byte
	//Removing the original nodes from the queue and pushing the new one
	unsigned size = Q.size();
	while (size>1)
	{
		Node* min1 = Q.top();
		Q.pop();
		Node* min2 = Q.top();
		Q.pop();
		Q.push(new Node(SENTINEL, min1->freq + min2->freq, min1, min2));
		--size;
	}
}

//Deletes tree recursively by given root of tree
void deleteHuffTree(Node* root)
{
	if (!root)
		return;
	deleteHuffTree(root->left);
	deleteHuffTree(root->right);
	delete root;
	root = nullptr;
}

//Loads tree from file by given root, file and number of vertecies
void loadTree(Node*& root, std::ifstream& fi, const unsigned int& N_VERT)
{
	static unsigned cnt = 0;
	uchar c = SENTINEL;
	unsigned f = FLAG;
	fi.read((char*)&c, sizeof(SENTINEL));
	fi.read((char*)&f, sizeof(FLAG));
	if (cnt == N_VERT || f == FLAG)
		return;
	cnt++;
	root = new Node(c, f);
	loadTree(root->left, fi, N_VERT);
	loadTree(root->right, fi, N_VERT);
}

//Saves tree to file by given root, file, number of bytes written to file and a variable to count number of vertecies
void saveTree(Node* root, ofstream& fo, unsigned& bytes, unsigned& vert)
{
	if (!root)
	{
		bytes += sizeof(SENTINEL) + sizeof(FLAG);
		fo.write((const char*)&SENTINEL, sizeof(SENTINEL));
		fo.write((const char*)&(FLAG), sizeof(FLAG));
		return;
	}
	vert++;
	bytes += sizeof(root->c) + sizeof(root->freq);
	fo.write((const char*)&root->c, sizeof(root->c));
	fo.write((const char*)&root->freq, sizeof(root->freq));
	saveTree(root->left, fo, bytes,vert);
	saveTree(root->right, fo, bytes,vert);
}

//Reads file and makes a map from byte to node, containing said byte
//If node with the current byte exists, we increment the frequency
void addCToMap(ifstream& file, CtoN& map)
{
	while (!file.eof())
	{
		uchar c;
		file.read((char*)&c, sizeof(c));
		if (file.eof())
			break;
		CtoN::iterator it = map.find(c);
		if (it == map.end())
			map.insert({ c, new Node(c) });
		else
			it->second->freq++;
	}
}

//Creates a map from byte to "code" by given root of tree, starting "code" string and a map
//The code is the path from the root of the tree to the leaf which stores the given byte
//Going left writes 0 to the path, going right writes 1
void createCodeMap(Node* root, string path, CtoCode& codes)
{
	if (!root)
		return;
	createCodeMap(root->left, path + "0", codes);
	if (!root->right && !root->left)
	{
		codes.insert({ root->c, path });
		return;
	}
	createCodeMap(root->right, path + "1", codes);
}

//Creates an archive file, containing all the files, requested by the user
void createArchive(const vector<fs::path>& paths, const unsigned& num_of_paths, const CtoCode& codes, Node* root)
{

	//The path of the archive is <requested_path>_archive
	//Archive files have a .huf extension
	//Structure of archive can be found in algorithm.hpp
	string archive_path = paths[0].string();
	unsigned arch_len = archive_path.length();
	if (archive_path[arch_len - 1] == '\\' || archive_path[arch_len - 1] == '/')
		archive_path[arch_len - 1] = ' ';
	archive_path += "_archive";
	fs::create_directories(archive_path);
	string archive_name = paths[0].filename().string() + ".huf";
	ofstream fo(archive_path + "/" + archive_name, std::ios::binary|std::ios::trunc);
	if (!fo.is_open())
	{
		cout << "Error opening file " << archive_name << " for writing!\n";
		return;
	}
	unsigned ARCHIVE_SIZE = 0;
	unsigned HEADER_SIZE = 0;
	unsigned num_of_vert = 0;
	fo.seekp(sizeof(ARCHIVE_SIZE) + sizeof(HEADER_SIZE) + sizeof(num_of_vert),std::ios::cur);
	saveTree(root, fo, HEADER_SIZE, num_of_vert);
	HEADER_SIZE += sizeof(ARCHIVE_SIZE) + sizeof(HEADER_SIZE) + sizeof(num_of_vert);
	ARCHIVE_SIZE += HEADER_SIZE;
	cout << "Header size:\t\t\t" << HEADER_SIZE << " bytes\n";
	for (unsigned i = 1; i < num_of_paths; ++i)
	{

		string file_name = paths[i].filename().string();
		string file_path = paths[i].parent_path().string();
		string relative_path = file_path.substr(paths[0].string().length());
		ifstream fi(file_path+"/"+file_name, std::ios::binary);
		if (!fi.is_open())
		{
			cout<<"Error opening file " << file_name << " for reading! File not added to archive!\n";
			continue;
		}
		unsigned compressed_size = 0;
		unsigned curr_header_size = 0;
		unsigned original_size = 0;
		fo.seekp(sizeof(curr_header_size) + sizeof(original_size), std::ios::cur);
		unsigned fn_len = file_name.length();
		unsigned rp_len = relative_path.length();
		fo.write((const char*)&fn_len, sizeof(fn_len));
		fo.write((const char*)&file_name[0], fn_len);
		fo.write((const char*)&rp_len, sizeof(rp_len));
		fo.write((const char*)&relative_path[0], rp_len);
		
		curr_header_size += sizeof(fn_len) + sizeof(rp_len) + fn_len + rp_len + sizeof(original_size)+sizeof(curr_header_size);
		uchar currbyte = 0;
		unsigned bitcount = 0;
		while (!fi.eof())
		{	
			uchar curr = 0;
			fi.read((char*)&curr, sizeof(curr));
			if (fi.eof())
				break;
			CtoCode::const_iterator it = codes.find(curr);
			if (it == codes.end())
			{
				cout << "Ran into a problem while archiving file " << file_name << "! Closing...\n";
				fi.close();
				fo.close();
				return;
			}
			string curr_path = it->second;
			unsigned len = curr_path.length();
			//Writing into a byte bit by bit using bitshifting
			for (unsigned i = 0; i < len; i++)
			{
				currbyte = currbyte << 1 | (curr_path[i] - '0');
				if (++bitcount == 8)
				{
					compressed_size += sizeof(currbyte);
					fo.write((const char*)(&currbyte), sizeof(currbyte));
					bitcount = 0;
					currbyte = 0;
				}
			}
		}
		//If we haven't filled a byte, we just add 0 bits at the end until it's full
		while (bitcount)
		{
			++bitcount;
			currbyte = currbyte << 1;
			if (bitcount == 8)
			{
				compressed_size += sizeof(currbyte);
				fo.write((char*)(&currbyte), sizeof(currbyte));
				bitcount = 0;
				currbyte = 0;
			}
		}
		fi.clear();
		fi.seekg(0, std::ios::end);
		original_size = fi.tellg();
		fo.seekp(-(int)(compressed_size + curr_header_size), std::ios::cur);
		fo.write((const char*)&original_size, sizeof(original_size));
		fo.write((const char*)&curr_header_size, sizeof(curr_header_size));
		fo.seekp(0, std::ios::end);

		cout << "Compressed file " << file_name << "\n";
		
		cout << "Size before compression:\t" << fi.tellg() << " bytes\n"
			<< "Size after compression:\t\t" << compressed_size << " bytes\n"
			<< "Individual header:\t\t" << curr_header_size << " bytes\n\n";	
		ARCHIVE_SIZE += compressed_size + curr_header_size;
		fi.close();
	}
	fo.seekp(0, std::ios::beg);
	fo.write((const char*)&ARCHIVE_SIZE, sizeof(ARCHIVE_SIZE));
	fo.write((const char*)&HEADER_SIZE, sizeof(HEADER_SIZE));
	fo.write((const char*)&num_of_vert, sizeof(num_of_vert));
	std::cout << "Total size of archive:\t\t" << ARCHIVE_SIZE << " bytes\n";
	fo.close();
}

//Unzips an archive, following the structure in algorithm.hpp
void unzipArchive(std::ifstream& fi, fs::path& path)
{
	cout << "Decompressing " << path.string() << endl;
	unsigned expected_size = 0;
	fi.read((char*)&expected_size, sizeof(expected_size));
	fi.seekg(0, std::ios::end);
	if (expected_size != fi.tellg())
	{
		cout << "Possible archive corruption!\n"
			<< "Expected file size\t\t" << expected_size
			<< "\nCurrent file size\t\t" << fi.tellg()<<endl;
		return;
	}
	fi.seekg(sizeof(expected_size), std::ios::beg);
	unsigned header_size = 0;
	fi.read((char*)&header_size, sizeof(header_size));
	unsigned num_of_vert = 0;
	fi.read((char*)&num_of_vert, sizeof(num_of_vert));
	unsigned end_of_prev = header_size;
	Node* huff_tree;
	loadTree(huff_tree, fi, num_of_vert);
	Node* root = huff_tree;
	if (header_size != fi.tellg())
	{
		cout << "Unexpected number of bytes read from archive header!\n";
		return;
	}
	while (!fi.eof())
	{
		unsigned file_sz = 0;
		fi.read((char*)&file_sz, sizeof(file_sz));
		unsigned file_header = 0;
		fi.read((char*)&file_header, sizeof(file_header));

		unsigned fn_len = 0;
		fi.read((char*)&fn_len, sizeof(fn_len));
		string compr_file_name;
		compr_file_name.resize(fn_len);
		fi.read((char*)&compr_file_name[0], fn_len);
		unsigned fp_len = 0;
		fi.read((char*)&fp_len, sizeof(fp_len));
		string file_path;
		file_path.resize(fp_len);
		fi.read((char*)&file_path[0], fp_len);
		if (fi.eof())
			break;
		fi.clear();
		end_of_prev += file_header;
		if (fi.tellg() != end_of_prev)
		{
			cout << "Unexpected number of bytes read from archived file "<<compr_file_name<<" header!\nProceeding with next file...\n";
			fi.seekg(file_sz, std::ios::cur);
			continue;
		}
		string unzip_path = path.parent_path().parent_path().string() + "/" + path.stem().string() + "_extracted/"+file_path;
		fs::create_directories(unzip_path);
		ofstream fo(unzip_path + "/" + compr_file_name, std::ios::binary|std::ios::trunc);
		if (!fo.is_open())
		{
			cout << "Error while opening file " << compr_file_name << " for writing!\nProceeding with next file...\n";
			fi.seekg(file_sz, std::ios::cur);
			continue;
		}
		uchar currbyte = 0;
		unsigned bitcount = 0;
		unsigned write = 0;
		for (; write < file_sz;)
		{
			//Reads a byte and uses each bit to follow a path through the loaded tree until it reaches a leaf
			fi.read((char*)&currbyte, sizeof(currbyte));
			for (int j = 7; j >= 0 && write < file_sz; j--)
			{
				if (huff_tree && ((currbyte >> j) & 1) == 0)
					huff_tree = huff_tree->left;
				else if (huff_tree)
					huff_tree = huff_tree->right;
				if (huff_tree && !huff_tree->left && !huff_tree->right)
				{
					fo.write((char*)&huff_tree->c, sizeof(huff_tree->c));
					huff_tree = root;
					write +=sizeof(huff_tree->c);
				}
			}
		}
		fo.close();
		cout << "Extracted file\t\t" << compr_file_name << "\nExpected size\t\t" << file_sz << "\nCurrent size\t\t" << write << endl<<endl;
		end_of_prev = fi.tellg();
	}
	deleteHuffTree(root);
}

//Reads each file and makes a map of bytes and nodes
void compressFiles(vector<fs::path>& paths)
{
	CtoN char_to_node;
	unsigned num_of_paths = paths.size();
	for (unsigned i = 1; i<num_of_paths;++i)
	{
		string file_name = paths[i].filename().string();
		string file_path = paths[i].parent_path().string();
		ifstream fi(paths[i].string(), std::ios::binary);
		if (fi.is_open())
		{
			addCToMap(fi, char_to_node);
			fi.close();
		}
		else
			cout << "Error opening file " << paths[i].string() << "for reading!\n";
	}
	NodeQ Q;
	for (CtoN::iterator it = char_to_node.begin(); it != char_to_node.end(); ++it)
		Q.push(it->second);
	createHuffTree(Q);
	CtoCode codes;
	Node* root = Q.top();
	Node* save = root;
	createCodeMap(root,"\0",codes);
	root = save;
	createArchive(paths, num_of_paths, codes, root);
	deleteHuffTree(root);
}


//Gives each of the requested files to the unzipArchive function
void decompressFiles(vector<fs::path>& paths)
{
	const unsigned NUM_OF_PATHS = paths.size();
	for (unsigned i = 1; i < NUM_OF_PATHS; i++)
	{
		std::string curr_path = paths[i].string();
		std::ifstream fi(curr_path, std::ios::binary);
		if (fi.is_open())
		{
			unzipArchive(fi, paths[i]);
			fi.close();
		}
		else
			std::cout << "Error while opening file " << paths[i].string() << " for reading!\n";
	}
}

//Checks if the path is viable and assigns it to given path variable
bool assignPath(fs::path& path, string& req_path)
{
	path.assign(req_path);
	if (path.parent_path().empty())
		path = fs::current_path() / req_path;

	if ((!path.has_extension()&&!fs::is_directory(path)) || (path.has_extension()&& !fs::is_regular_file(path)))
	{
		if (path.has_extension() && path.stem() == "*")
			return true;
		std::cout << "Requested path or file does not exist!\n";
		return false;
	}
	return true;
}

//Finds all sub-directories, suitable for compression and pushes them to a vector
bool addPathsToCompress(vector<fs::path>& paths)
{
	fs::path first = paths[0];
	unsigned num = 0;
	for (const fs::directory_entry& p : fs::recursive_directory_iterator(first))
	{
		if (p.path().has_extension())
		{
			paths.push_back(p.path());
			num++;
		}
	}
	if (!num)
	{
		cout << "Path is empty!\n";
		return false;
	}
	return true;
}

//Finds all sub-directories and files, suitable for decompression and pushes them to a vector
bool addPathsToDecompress(vector<fs::path>& paths)
{
	fs::path first = paths[0];
	unsigned num = 0;
	for (const fs::directory_entry& p : fs::recursive_directory_iterator(first))
	{
		if (p.path().has_extension()&&p.path().extension()==".huf")
		{
			paths.push_back(p.path());
			num++;
		}
	}
	if (!num)
	{
		cout << "Path is empty or has no .huf files!\n";
		return false;
	}
	return true;
}

//Initiates compression
void compress(string& req_path)
{
	fs::path path;
	if (!assignPath(path, req_path))
		return;
	vector<fs::path> all_paths;
	if (path.has_extension())
	{
		all_paths.push_back(path.parent_path());
		if (path.stem() == "*")
		{
			for (const fs::directory_entry& p : fs::directory_iterator(path.parent_path()))
			{
				if (p.path().extension() == path.extension())
					all_paths.push_back(p.path());
			}
		}
		else
			all_paths.push_back(path);
	}
	else
	{
		all_paths.push_back(path);
		if (!addPathsToCompress(all_paths))
			return;
	}
	compressFiles(all_paths);
}

//Initiates decompression
void decompress(string& req_path)
{
	fs::path path;
	if(!assignPath(path, req_path))
		return;
	vector<fs::path>all_paths;
	if (path.has_extension() && path.extension() == ".huf")
	{
		all_paths.push_back(path.parent_path());
		all_paths.push_back(path);
	}
	else if (path.has_extension())
	{
		cout << "File must have a .huf extension!\n";
		return;
	}
	else
	{
		all_paths.push_back(path);
		if (!addPathsToDecompress(all_paths))
			return;
	}
	decompressFiles(all_paths);
}