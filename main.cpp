#include <iostream>
#include <string>
#include "commands.hpp"
#include "node.hpp"
#include "algorithm.hpp"

using std::cout, std::cin, std::endl, std::string;

//Get input and execute commands
int main()
{
	cout << "Enter one of the commands bellow:\n"
		<< "zip <path_to_file>\n"
		<< "unzip <path_to_file>\n"
		<< "exit\n";
	do
	{
		cout << "\n> ";
		CommandLine cl;
		std::getline(cin, cl.input);
		string command = cl.extractWord();
		if (!command.compare("zip"))
		{
			string path = cl.extractRest();
			compress(path);
		}
		else if (!command.compare("unzip"))
		{
			string path = cl.extractRest();
			decompress(path);
		}
		else if (!command.compare("exit"))
			break;
		else
			cout << "Unknown command!\n";
	} while (true);
	
	return 0;
}