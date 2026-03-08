/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RobotomyRequestForm.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:41:18 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 14:53:08 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RobotomyRequestForm.hpp"
#include <iostream>
#include <cstdlib>

RobotomyRequestForm::RobotomyRequestForm(std::string const& target)
	: AForm("RobotomyRequestForm", 72, 45), _target(target)
{}

RobotomyRequestForm::RobotomyRequestForm(RobotomyRequestForm const& other)
	: AForm(other), _target(other._target)
{}

RobotomyRequestForm::~RobotomyRequestForm(void)
{}

RobotomyRequestForm& RobotomyRequestForm::operator=(RobotomyRequestForm const& other)
{
	if (this != &other)
	{
		AForm::operator=(other);
		_target = other._target;
	}
	return *this;
}

void RobotomyRequestForm::execute(Bureaucrat const& executor) const
{
	checkExecute(executor);

	std::cout << "Drrrrrr... BZZZZZT... drilling noises..." << std::endl;
	if (std::rand() % 2)
		std::cout << _target << " has been robotomized successfully!" << std::endl;
	else
		std::cout << _target << " robotomy failed." << std::endl;
}
