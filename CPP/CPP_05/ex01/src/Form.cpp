/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 18:46:29 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/06 19:51:55 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Form.hpp"
#include <iostream>

Form::Form(void)
	: _name("default"), _signed(false), _signGrade(150), _execGrade(150) {}

Form::Form(std::string const& name, int signGrade, int execGrade)
	: _name(name), _signed(false), _signGrade(signGrade), _execGrade(execGrade)
{
	if (signGrade < 1 || execGrade < 1)
		throw Form::GradeTooHighException();
	if (signGrade > 150 || execGrade > 150)
		throw Form::GradeTooLowException();
}

Form::Form(Form const& other)
	: _name(other._name), _signed(other._signed),
	_signGrade(other._signGrade), _execGrade(other._execGrade)
{}

Form::~Form(void)
{}

Form& Form::operator=(Form const& other)
{
	if (this != &other)
		_signed = other._signed;
	return *this;
}

std::string const& Form::getName(void) const      { return _name; }
bool               Form::getSigned(void) const    { return _signed; }
int                Form::getSignGrade(void) const { return _signGrade; }
int                Form::getExecGrade(void) const { return _execGrade; }

void Form::beSigned(Bureaucrat const& b)
{
	if (b.getGrade() > _signGrade)
		throw Form::GradeTooLowException();
	_signed = true;
}

char const* Form::GradeTooHighException::what(void) const throw()
{
	return "Form: grade is too high";
}

char const* Form::GradeTooLowException::what(void) const throw()
{
	return "Form: grade is too low";
}

std::ostream& operator<<(std::ostream& os, Form const& form)
{
	os << "Form: "       << form.getName()
		<< " | signed: "  << (form.getSigned() ? "yes" : "no")
		<< " | signGrade: " << form.getSignGrade()
		<< " | execGrade: " << form.getExecGrade();
	return os;
}
