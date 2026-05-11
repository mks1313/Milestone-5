/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 18:45:15 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/11 20:04:20 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "whatever.hpp"
#include "Colors.hpp"

#include <iostream>
#include <string>

template<typename T>
void runTest()
{
	T a;
	T b;

	std::cout << BLUE
			  << "\nEnter first value: "
			  << RESET;
	std::cin >> a;

	std::cout << BLUE
			  << "Enter second value: "
			  << RESET;
	std::cin >> b;

	std::cout << BOLD << GREEN
			  << "\nBefore swap:"
			  << RESET << std::endl;

	std::cout << "a = " << a
			  << ", b = " << b
			  << std::endl;

	::swap(a, b);

	std::cout << BOLD << MAGENTA
			  << "\nAfter swap:"
			  << RESET << std::endl;

	std::cout << "a = " << a
			  << ", b = " << b
			  << std::endl;

	std::cout << CYAN
			  << "\nmin(a, b): "
			  << RESET
			  << ::min(a, b)
			  << std::endl;

	std::cout << CYAN
			  << "max(a, b): "
			  << RESET
			  << ::max(a, b)
			  << std::endl;
}

int main(void)
{
	int choice;

	std::cout << BOLD << YELLOW
			  << "========== TEMPLATE TESTER ==========\n"
			  << RESET << std::endl;

	std::cout << GREEN
			  << "1. int\n"
			  << CYAN
			  << "2. float\n"
			  << MAGENTA
			  << "3. string\n"
			  << RESET << std::endl;

	std::cout << BLUE
			  << "Choose type: "
			  << RESET;

	std::cin >> choice;

	if (choice == 1)
		runTest<int>();
	else if (choice == 2)
		runTest<float>();
	else if (choice == 3)
		runTest<std::string>();
	else
	{
		std::cout << BOLD << RED
				  << "\nInvalid choice!\n"
				  << RESET;
	}

	return (0);
}

/*
   int main( void  ) {
   int a = 2;
   int b = 3;
   ::swap( a, b  );
   std::cout << "a = " << a << ", b = " << b << std::endl;
   std::cout << "min( a, b  ) = " << ::min( a, b  ) << std::endl;
   std::cout << "max( a, b  ) = " << ::max( a, b  ) << std::endl;
   std::string c = "chaine1";
   std::string d = "chaine2";
   ::swap(c, d);
   std::cout << "c = " << c << ", d = " << d << std::endl;
   std::cout << "min( c, d  ) = " << ::min( c, d  ) << std::endl;
   std::cout << "max( c, d  ) = " << ::max( c, d  ) << std::endl;
   return 0;

   }*/
