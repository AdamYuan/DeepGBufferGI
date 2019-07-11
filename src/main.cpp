#include <iostream>
#include "Application.hpp"

int main(int argc, char **argv)
{
	Application app{};
	if(argc == 2) app.LoadScene(argv[1]);
	app.Run();
	return 0;
}