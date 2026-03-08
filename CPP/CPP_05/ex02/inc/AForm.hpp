/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 17:07:06 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 12:41:05 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AFORM_HPP
# define AFORM_HPP

# include <iosfwd>
# include <string>
# include <exception>

# include "Bureaucrat.hpp"

class AForm
{
private:
	const std::string _name;
	bool _signed;
	const int _signGrade;
	const int _execGrade;

public:
	AForm(void);
	AForm(std::string const& name, int signGrade, int execGrade);
	AForm(AForm const& other);
	virtual ~AForm(void);

	AForm& operator=(AForm const& other);

	std::string const& getName(void) const;
	bool getSigned(void) const;
	int  getSignGrade(void) const;
	int  getExecGrade(void) const;

	void beSigned(Bureaucrat const& b);

	class GradeTooHighException : public std::exception{
		public:
			virtual char const *what(void) const throw();
	};

	class GradeTooLowException : public std::exception{
		public:
			virtual char const *what(void) const throw();
	};

	class FormNotSignedException : public std::exception{
		public:
			virtual char const *what(void) const throw();
	};

	void checkExecute(Bureaucrat const& executor) const;

	virtual void execute(Bureaucrat const & executor) const = 0;

};

std::ostream &operator<<(std::ostream &str, AForm const& form);

#endif
