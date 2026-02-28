/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:18:49 by mmarinov          #+#    #+#             */
/*   Updated: 2026/02/28 16:48:55 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Bureaucrat.hpp"

int main(void)
{
	try {
		Bureaucrat a("Terry", 2);
		std::cout << a << std::endl;

		a.gradeUp();
		std::cout << a << std::endl;

		a.gradeUp();
	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}

	std::cout << "-------- Copy constructor ----------" << std::endl;
	try {
		Bureaucrat Bob("Bob", 100);
		Bureaucrat Dill(Bob);
		Bureaucrat Bill("Bill", 130);
		Bill = Dill;
		std::cout << Bob << std::endl;
		std::cout << Dill << std::endl;
		std::cout << Bill << std::endl;
	}

	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	std::cout << "------ Now Limits ----------" << std::endl;

	try {
		Bureaucrat b("Jim", 150);
		std::cout << b << std::endl;

		b.gradeDown();
	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}

	std::cout << "-----Failes constructors-----------" << std::endl;

	try {
		Bureaucrat c("ErrorHigh", 0); // ERROR
	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}

	try {
		Bureaucrat d("ErrorLow", 200); // ERROR
	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	
	std::cout << "--------Normal-------------" << std::endl;

	try {
		Bureaucrat norm("Normal", 134);
		std::cout << norm << std::endl;
		norm.gradeDown();
		std::cout << norm << std::endl;
		norm.gradeDown();
		std::cout << norm << std::endl;
		norm.gradeUp();
		std::cout << norm << std::endl;
	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
