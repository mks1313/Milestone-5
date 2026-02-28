/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 17:07:06 by mmarinov          #+#    #+#             */
/*   Updated: 2026/02/28 17:36:25 by mmarinov         ###   ########.fr       */
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
	const int _grade;
	const int _exec_grade;

public:
	Form(void);
	Form(std::string const& name, bool signed, const int grade, const int _exec_grade);
	Form(Form const& cpy);
	~Form(void);

	Form& operator=(Form const& cpy);

	std::string const& getName(void) const;
	bool const getSigned(void) const;
	const int getGrade(void) const;
	const int getExecGrade(void) const;

	int beSigned(Bureaucrat &b) const;
	bool signForm(void) const;

	class GradeTooHighException : public std::exception{
		public:
			virtual char const *what(void) const throw();
	};

	class GradeTooHighException : public std::exception{
		public:
			virtual char const *what(void) const throw();
	};

};

std::ostream &operator<<(std::ostream &str, const& form);

#endif
