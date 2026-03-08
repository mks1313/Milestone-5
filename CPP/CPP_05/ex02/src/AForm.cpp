/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 18:46:29 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 13:47:43 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AForm.hpp"
#include <iostream>

AForm::AForm(void)
	: _name("default"), _signed(false), _signGrade(150), _execGrade(150) {}

AForm::AForm(std::string const& name, int signGrade, int execGrade)
	: _name(name), _signed(false), _signGrade(signGrade), _execGrade(execGrade)
{
	if (signGrade < 1 || execGrade < 1)
		throw AForm::GradeTooHighException();
	if (signGrade > 150 || execGrade > 150)
		throw AForm::GradeTooLowException();
}

AForm::AForm(AForm const& other)
	: _name(other._name), _signed(other._signed),
	_signGrade(other._signGrade), _execGrade(other._execGrade)
{}

AForm::~AForm(void) {}

AForm& AForm::operator=(AForm const& other){
	if (this != &other)
		_signed = other._signed;
	return *this;
}

std::string const& AForm::getName(void) const      { return _name; }
bool               AForm::getSigned(void) const    { return _signed; }
int                AForm::getSignGrade(void) const { return _signGrade; }
int                AForm::getExecGrade(void) const { return _execGrade; }

void AForm::beSigned(Bureaucrat const& b){
	if (b.getGrade() > _signGrade)
		throw AForm::GradeTooLowException();
	_signed = true;
}

char const* AForm::GradeTooHighException::what(void) const throw(){
	return "AForm: grade is too high";
}

char const* AForm::GradeTooLowException::what(void) const throw(){
	return "AForm: grade is too low";
}

char const* AForm::FormNotSignedException::what(void) const throw(){
	return "AForm: form is not signed";
}

void AForm::checkExecute(Bureaucrat const& executor) const {
	if (!getSigned())
		throw FormNotSignedException();
	if (executor.getGrade() > getExecGrade())
		throw GradeTooLowException();
}

std::ostream& operator<<(std::ostream& os, AForm const& form)
{
	os << "AForm: "       << form.getName()
		<< " | signed: "  << (form.getSigned() ? "yes" : "no")
		<< " | signGrade: " << form.getSignGrade()
		<< " | execGrade: " << form.getExecGrade();
	return os;
}
