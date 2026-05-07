#include "Functions.hpp"
#include "Base.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include "Colors.hpp"
#include <ctime>
#include <cstdlib>
#include <iostream>

int main() {
	srand(time(NULL));

	std::cout << BOLD << GREEN << "Manual Test" << RESET << std::endl;
	Base* a = new A();
	Base* b = new B();
	Base* c = new C();

	identify(a);
	identify(*a);
	identify(b);
	identify(*b);
	identify(c);
	identify(*c);
	identify(NULL);

	delete a;
	delete b;
	delete c;

	std::cout << BOLD << BLUE << "Generate Test" << RESET << std::endl;
	for (int i = 0; i < 10; i++) {
	Base* obj = generate();
	identify(obj);
	identify(*obj);
	delete obj;
	}
	return 0;
}
