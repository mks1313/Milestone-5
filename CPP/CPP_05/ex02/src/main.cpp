/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:18:49 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:05:13 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "AForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

#define RESET "\033[0m"
#define BOLD "\033[1m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define OK GREEN "[OK] " RESET
#define KO RED "[KO] " RESET
#define INFO YELLOW "[..] " RESET

static void printHeader(const std::string &title)
{
	std::cout << "\n"
						<< CYAN << BOLD;
	std::cout << "====================================================\n";
	std::cout << title << "\n";
	std::cout << "====================================================\n";
	std::cout << RESET;
}

static void printSub(const std::string &title)
{
	std::cout << "\n"
						<< MAGENTA << BOLD << "---- " << title << " ----" << RESET << "\n";
}

int main(void)
{
	std::srand(std::time(NULL));

	printHeader("CPP05 EX02 - FULL TEST");

	printSub("1. BASIC BUREAUCRAT TEST");
	try
	{
		Bureaucrat boss("Boss", 1);
		Bureaucrat mid("Mid", 50);
		Bureaucrat low("Low", 150);

		std::cout << BLUE << boss << RESET << std::endl;
		std::cout << BLUE << mid << RESET << std::endl;
		std::cout << BLUE << low << RESET << std::endl;
		std::cout << OK << "Bureaucrats created successfully" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("2. EXECUTE UNSIGNED FORM");
	try
	{
		Bureaucrat boss("Boss", 1);
		ShrubberyCreationForm shrub("unsigned_garden");

		std::cout << BLUE << boss << RESET << std::endl;
		std::cout << MAGENTA << shrub << RESET << std::endl;

		std::cout << INFO << "Trying to execute unsigned form..." << std::endl;
		boss.executeForm(shrub);

		std::cout << OK << "executeForm handled the unsigned case" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << OK << "Expected exception caught: " << e.what() << std::endl;
	}

	printSub("3. SIGN FAIL - GRADE TOO LOW");
	try
	{
		Bureaucrat low("Low", 150);
		RobotomyRequestForm robot("Bender");

		std::cout << BLUE << low << RESET << std::endl;
		std::cout << MAGENTA << robot << RESET << std::endl;

		std::cout << INFO << "Trying to sign with low bureaucrat..." << std::endl;
		low.signForm(robot);

		std::cout << OK << "signForm handled the low grade case" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << OK << "Expected exception caught: " << e.what() << std::endl;
	}

	printSub("4. SIGN OK, EXECUTE FAIL");
	try
	{
		Bureaucrat signer("Signer", 1);
		Bureaucrat weakExecutor("WeakExecutor", 140);
		ShrubberyCreationForm shrub("park");

		std::cout << BLUE << signer << RESET << std::endl;
		std::cout << BLUE << weakExecutor << RESET << std::endl;
		std::cout << MAGENTA << shrub << RESET << std::endl;

		std::cout << INFO << "Signing form..." << std::endl;
		signer.signForm(shrub);
		std::cout << GREEN << shrub << RESET << std::endl;

		std::cout << INFO << "Trying to execute with insufficient grade..." << std::endl;
		weakExecutor.executeForm(shrub);

		std::cout << OK << "executeForm handled the low grade case" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << OK << "Expected exception caught: " << e.what() << std::endl;
	}

	printSub("5. SHRUBBERY SUCCESS");
	try
	{
		Bureaucrat boss("Boss", 1);
		ShrubberyCreationForm shrub("home");

		std::cout << MAGENTA << shrub << RESET << std::endl;

		std::cout << INFO << "Signing shrubbery form..." << std::endl;
		boss.signForm(shrub);
		std::cout << GREEN << shrub << RESET << std::endl;

		std::cout << INFO << "Executing shrubbery form..." << std::endl;
		boss.executeForm(shrub);

		std::cout << OK << "Shrubbery executed. Check file: home_shrubbery" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("6. ROBOTOMY MULTIPLE TIMES");
	try
	{
		Bureaucrat boss("Boss", 1);
		RobotomyRequestForm robot("Marvin");

		std::cout << MAGENTA << robot << RESET << std::endl;

		std::cout << INFO << "Signing robotomy form..." << std::endl;
		boss.signForm(robot);
		std::cout << GREEN << robot << RESET << std::endl;

		std::cout << INFO << "Executing robotomy 5 times..." << std::endl;
		boss.executeForm(robot);
		boss.executeForm(robot);
		boss.executeForm(robot);
		boss.executeForm(robot);
		boss.executeForm(robot);

		std::cout << OK << "Robotomy multiple execution test done" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("7. PRESIDENTIAL PARDON SUCCESS");
	try
	{
		Bureaucrat president("President", 1);
		PresidentialPardonForm pardon("Arthur Dent");

		std::cout << MAGENTA << pardon << RESET << std::endl;

		std::cout << INFO << "Signing pardon form..." << std::endl;
		president.signForm(pardon);
		std::cout << GREEN << pardon << RESET << std::endl;

		std::cout << INFO << "Executing pardon form..." << std::endl;
		president.executeForm(pardon);

		std::cout << OK << "Presidential pardon test passed" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("8. POLYMORPHISM TEST WITH AForm*");
	try
	{
		Bureaucrat boss("Boss", 1);

		AForm *forms[3];
		forms[0] = new ShrubberyCreationForm("forest");
		forms[1] = new RobotomyRequestForm("R2D2");
		forms[2] = new PresidentialPardonForm("Ford Prefect");

		for (int i = 0; i < 3; i++)
		{
			std::cout << WHITE << "Form " << i << ": " << *forms[i] << RESET << std::endl;

			std::cout << INFO << "Signing..." << std::endl;
			boss.signForm(*forms[i]);

			std::cout << INFO << "Executing..." << std::endl;
			boss.executeForm(*forms[i]);

			std::cout << OK << "Polymorphic execution successful" << std::endl;
		}

		for (int i = 0; i < 3; i++)
			delete forms[i];
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("9. COPY CONSTRUCTOR + ASSIGNMENT TEST");
	try
	{
		Bureaucrat boss("Boss", 1);

		ShrubberyCreationForm original("copy_test");
		boss.signForm(original);

		ShrubberyCreationForm copy(original);
		ShrubberyCreationForm assigned("temp");
		assigned = original;

		std::cout << WHITE << "Original : " << original << RESET << std::endl;
		std::cout << WHITE << "Copy     : " << copy << RESET << std::endl;
		std::cout << WHITE << "Assigned : " << assigned << RESET << std::endl;

		std::cout << INFO << "Executing copy..." << std::endl;
		boss.executeForm(copy);

		std::cout << INFO << "Executing assigned..." << std::endl;
		boss.executeForm(assigned);

		std::cout << OK << "Copy and assignment test passed" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("10. EDGE CASE - EXACT GRADE SIGN AND EXECUTE");
	try
	{
		Bureaucrat signExact("SignExact", 145);
		Bureaucrat execExact("ExecExact", 137);
		ShrubberyCreationForm shrub("exact_case");

		std::cout << BLUE << signExact << RESET << std::endl;
		std::cout << BLUE << execExact << RESET << std::endl;
		std::cout << MAGENTA << shrub << RESET << std::endl;

		std::cout << INFO << "Signing with exact required grade..." << std::endl;
		signExact.signForm(shrub);

		std::cout << INFO << "Executing with exact required grade..." << std::endl;
		execExact.executeForm(shrub);

		std::cout << OK << "Exact grade boundaries work correctly" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << KO << e.what() << std::endl;
	}

	printSub("11. EDGE CASE - EXEC GRADE TOO LOW BY ONE");
	try
	{
		Bureaucrat signer("Signer", 1);
		Bureaucrat almost("Almost", 138);
		ShrubberyCreationForm shrub("fail_by_one");

		signer.signForm(shrub);

		std::cout << INFO << "Trying execute with grade 138 while required is 137..." << std::endl;
		almost.executeForm(shrub);

		std::cout << OK << "executeForm handled the fail-by-one case" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << OK << "Expected exception caught: " << e.what() << std::endl;
	}

	printHeader("END OF TESTS");
	return 0;
}
