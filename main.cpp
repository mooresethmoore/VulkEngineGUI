
#include "first_app_GUI.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	lve::FirstAppGUI app{};

	try {
		app.run();

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
