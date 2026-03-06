/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:18:49 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/06 19:53:58 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "Form.hpp"
#include <iostream>

int main()
{
	std::cout << "---- BASIC FORM TEST ----\n";

	try
	{
		Bureaucrat bob("Bob", 50);
		Form taxForm("Taxes", 45, 30);

		std::cout << bob << std::endl;
		std::cout << taxForm << std::endl;

		std::cout << "\nBob tries to sign form\n";
		taxForm.beSigned(bob);

		std::cout << taxForm << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	std::cout << "\n---- SIGN FAIL TEST ----\n";

	try
	{
		Bureaucrat tom("Tom", 100);
		Form permit("Permit", 50, 20);

		std::cout << tom << std::endl;
		std::cout << permit << std::endl;

		std::cout << "\nTom tries to sign form\n";
		permit.beSigned(tom);
	}
	catch (std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	std::cout << "\n---- CONSTRUCTOR ERROR TEST ----\n";

	try
	{
		Form invalid("BadForm", 0, 20);
	}
	catch (std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
