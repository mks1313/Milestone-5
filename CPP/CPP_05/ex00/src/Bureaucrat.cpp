/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bureaucrat.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 14:03:48 by mmarinov          #+#    #+#             */
/*   Updated: 2026/02/28 16:22:39 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ostream>
#include "Bureaucrat.hpp"

Bureaucrat::Bureaucrat(void) : _name("default"), _grade(150) {}

Bureaucrat::Bureaucrat(std::string const &name, int grade) : _name(name), _grade(grade) {
	if (this->_grade < 1) {
		throw (Bureaucrat::GradeTooHighException());
	}
	else if (this->_grade > 150) {
		throw (Bureaucrat::GradeTooLowException());
	}
}

Bureaucrat::Bureaucrat(Bureaucrat const& copy) : _name(copy._name), _grade(copy._grade) {}

Bureaucrat::~Bureaucrat(void) {}

Bureaucrat &Bureaucrat::operator=(const Bureaucrat& copy) {
	this->_grade = copy._grade;
	return *this;
}

std::string const& Bureaucrat::getName(void) const { return this->_name; }

int Bureaucrat::getGrade(void) const { return this->_grade; }

void Bureaucrat::gradeUp(void) {
	if (this->_grade <= 1)
		throw(Bureaucrat::GradeTooHighException());
	this->_grade--;
}

void Bureaucrat::gradeDown(void) {
	if (this->_grade >= 150)
		throw(Bureaucrat::GradeTooLowException());
	this->_grade++;
}

char const *Bureaucrat::GradeTooHighException::what(void) const throw() {
	return "Grade is too high";
}

char const *Bureaucrat::GradeTooLowException::what(void) const throw() {
	return "Grade is too low";
}

std::ostream &operator<<(std::ostream &str, Bureaucrat const& bureaucrat) {
	return str << bureaucrat.getName() << ", bureaucrat grade " << bureaucrat.getGrade() << ".";
}
