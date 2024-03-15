#pragma once
#include <string>
using std::string;

struct CommandLine
{
	unsigned ind = 0;
	string input;

	//Removes ' '
	void clearWhitespaces();
	//Returns one word (until we get ' ' or the string ends)
	string extractWord();
	//Returns the rest of the input
	string extractRest();
};