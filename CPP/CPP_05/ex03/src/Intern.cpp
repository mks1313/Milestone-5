/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 15:03:07 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:04:41 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Intern.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"
#include <iostream>

Intern::Intern(void) {}
Intern::Intern(Intern const& other) { (void)other; }
Intern::~Intern(void) {}
Intern& Intern::operator=(Intern const& other) { (void)other; return *this; }

AForm* Intern::makeForm(std::string const& formName, std::string const& target) const
{
	std::string const names[3] = {
		"ShrubberyCreationForm",
		"RobotomyRequestForm",
		"PresidentialPardonForm"
	};

	int i = 0;
	while (i < 3 && names[i] != formName)
		++i;

	switch (i)
	{
		case 0: return new ShrubberyCreationForm(target);
		case 1: return new RobotomyRequestForm(target);
		case 2: return new PresidentialPardonForm(target);
		default:
			std::cerr << "Intern: unknown form \"" << formName << "\"" << std::endl;
			return NULL;
	}
}
