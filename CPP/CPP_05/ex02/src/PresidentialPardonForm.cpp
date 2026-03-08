/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PresidentialPardonForm.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:57:27 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 14:57:53 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PresidentialPardonForm.hpp"
#include <iostream>

PresidentialPardonForm::PresidentialPardonForm(std::string const& target)
	: AForm("PresidentialPardonForm", 25, 5), _target(target)
{}

PresidentialPardonForm::PresidentialPardonForm(PresidentialPardonForm const& other)
	: AForm(other), _target(other._target)
{}

PresidentialPardonForm::~PresidentialPardonForm(void)
{}

PresidentialPardonForm& PresidentialPardonForm::operator=(PresidentialPardonForm const& other)
{
	if (this != &other)
	{
		AForm::operator=(other);
		_target = other._target;
	}
	return *this;
}

void PresidentialPardonForm::execute(Bureaucrat const& executor) const
{
	checkExecute(executor);
	std::cout << _target << " has been pardoned by Zaphod Beeblebrox." << std::endl;
}
