#include "commands.hpp"
void CommandLine::clearWhitespaces()
{
	while (input[ind] == ' ')
		ind++;
}

string CommandLine::extractWord()
{
	clearWhitespaces();
	std::string word;
	while (input[ind] != ' ' && input[ind] != '\0')
		word += input[ind++];
	return word;
}

string CommandLine::extractRest()
{
	clearWhitespaces();
	return input.substr(ind);
}