/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 17:07:06 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/06 19:41:04 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FORM_HPP
# define FORM_HPP

# include <iosfwd>
# include <string>
# include <exception>

# include "Bureaucrat.hpp"

class Form
{
private:
	const std::string _name;
	bool _signed;
	const int _signGrade;
	const int _execGrade;

public:
	Form(void);
	Form(std::string const& name, int signGrade, int execGrade);
	Form(Form const& other);
	~Form(void);

	Form& operator=(Form const& other);

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

};

std::ostream &operator<<(std::ostream &str, Form const& form);

#endif
