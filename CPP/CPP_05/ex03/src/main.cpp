/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:18:49 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:08:43 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Bureaucrat.hpp"
#include "AForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"
#include "Intern.hpp"

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

static void separator(std::string const& title)
{
	std::cout << "\n" << BOLD << CYAN
		<< "===== " << title << " ====="
		<< RESET << std::endl;
}

static void ok(std::string const& msg)
{
	std::cout << GREEN << "[OK]  " << RESET << msg << std::endl;
}

static void catchErr(std::string const& ctx, std::exception& e)
{
	std::cerr << RED << "[CATCH " << ctx << "] " << RESET << e.what() << std::endl;
}

int main(void)
{
	std::srand(std::time(NULL));

	/* ---- Bureaucrat: grades invalidos ---- */
	separator("Bureaucrat invalid grades");
	try { Bureaucrat b("TooHigh", 0); }
	catch (std::exception& e) { catchErr("grade 0", e); }

	try { Bureaucrat b("TooLow", 151); }
	catch (std::exception& e) { catchErr("grade 151", e); }

	/* ---- ShrubberyCreationForm ---- */
	separator("ShrubberyCreationForm");
	try
	{
		Bureaucrat tooWeak("TooWeak", 146);
		Bureaucrat exact("Exact", 145);
		Bureaucrat strong("Strong", 1);
		ShrubberyCreationForm f("home");

		std::cout << YELLOW << f << RESET << std::endl;

		// grade 146 > signGrade 145 → debe fallar
		tooWeak.signForm(f);

		// form sigue sin firmar → debe fallar
		strong.executeForm(f);

		// grade 145 == signGrade 145 → debe firmar
		exact.signForm(f);
		std::cout << YELLOW << f << RESET << std::endl;

		// grade 137 == execGrade 137 → debe ejecutar
		Bureaucrat execExact("ExecExact", 137);
		execExact.executeForm(f);
		ok("home_shrubbery created");
	}
	catch (std::exception& e) { catchErr("unexpected", e); }

	/* ---- RobotomyRequestForm ---- */
	separator("RobotomyRequestForm");
	try
	{
		Bureaucrat signer("Signer", 70);
		Bureaucrat exec("Exec", 45);
		RobotomyRequestForm f("Bender");

		std::cout << YELLOW << f << RESET << std::endl;

		try { exec.executeForm(f); }
		catch (std::exception& e) { catchErr("exec unsigned", e); }

		signer.signForm(f);

		Bureaucrat weakExec("WeakExec", 46);
		try { weakExec.executeForm(f); }
		catch (std::exception& e) { catchErr("exec low grade", e); }

		std::cout << MAGENTA << "5 robotomy attempts:" << RESET << std::endl;
		for (int i = 0; i < 5; ++i)
			exec.executeForm(f);
	}
	catch (std::exception& e) { catchErr("unexpected", e); }

	/* ---- PresidentialPardonForm ---- */
	separator("PresidentialPardonForm");
	try
	{
		Bureaucrat signer("Signer", 24);
		Bureaucrat exec("Exec", 5);
		PresidentialPardonForm f("Arthur Dent");

		std::cout << YELLOW << f << RESET << std::endl;

		try { exec.executeForm(f); }
		catch (std::exception& e) { catchErr("exec unsigned", e); }

		signer.signForm(f);

		Bureaucrat weakExec("WeakExec", 6);
		try { weakExec.executeForm(f); }
		catch (std::exception& e) { catchErr("exec low grade", e); }

		exec.executeForm(f);
	}
	catch (std::exception& e) { catchErr("unexpected", e); }

	/* ---- Intern::makeForm ---- */
	separator("Intern makeForm - validos");
	{
		Intern intern;
		Bureaucrat boss("Boss", 1);

		std::string forms[3] = {
			"ShrubberyCreationForm",
			"RobotomyRequestForm",
			"PresidentialPardonForm"
		};
		std::string targets[3] = { "garden", "Marvin", "Ford" };

		for (int i = 0; i < 3; ++i)
		{
			AForm* f = intern.makeForm(forms[i], targets[i]);
			if (f)
			{
				std::cout << YELLOW << *f << RESET << std::endl;
				boss.signForm(*f);
				boss.executeForm(*f);
				ok(forms[i] + " executed");
				delete f;
			}
		}
	}

	separator("Intern makeForm - nombre invalido");
	{
		Intern intern;
		AForm* f = intern.makeForm("NonExistentForm", "target");
		if (!f)
			std::cout << GREEN << "[OK]  " << RESET
				<< "makeForm returned NULL as expected" << std::endl;
	}

	separator("Fin");
	return 0;
}
