/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShrubberyCreationForm.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 12:33:19 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:11:58 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ShrubberyCreationForm.hpp"
#include <fstream>

ShrubberyCreationForm::ShrubberyCreationForm(std::string const& target)
	: AForm("ShrubberyCreationForm", 145, 137), _target(target)
{}

ShrubberyCreationForm::ShrubberyCreationForm(ShrubberyCreationForm const& other)
	: AForm(other), _target(other._target)
{}

ShrubberyCreationForm::~ShrubberyCreationForm(void)
{}

ShrubberyCreationForm& ShrubberyCreationForm::operator=(ShrubberyCreationForm const& other)
{
	if (this != &other)
	{
		AForm::operator=(other);
		_target = other._target;
	}
	return *this;
}

void ShrubberyCreationForm::execute(Bureaucrat const& executor) const
{
	checkExecute(executor);

	std::string const filename = _target + "_shrubbery";
	std::ofstream ofs(filename.c_str());
	if (!ofs) return;
	ofs << "          *\n"
	       "         ***\n"
	       "        *****\n"
	       "       *******\n"
	       "      *********\n"
	       "     ***********\n"
	       "    *************\n"
	       "         | |\n"
	       "          *\n"
	       "         ***\n"
	       "        *****\n"
	       "       *******\n"
	       "      *********\n"
	       "     ***********\n"
	       "    *************\n"
	       "         | |\n";
}
