#include <iostream>
#include "PrintTool.h"

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		PrintTool::Run(argv[1]);
	}
	else
	{
		std::cout << "PRINTTOOL [in]\n\n";
		std::cout << "\t[in] - Specifies a directory or file to print. If a directory is specified, all valid files within the directory will be printed.\n\n";
	}
	return 0;
}