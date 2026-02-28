/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bureaucrat.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/28 12:43:37 by mmarinov          #+#    #+#             */
/*   Updated: 2026/02/28 13:51:29 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUREAUCRAT_HPP
# define BUREAUCRAT_HPP

# include <iosfwd>
# include <exception>
# include <string>

class Bureaucrat
{
	private:
			const std::string _name;
			int _grade;
	public:
			Bureaucrat(void);
			Bureaucrat(std::string const& name, int grade);
			Bureaucrat(Bureaucrat const& copy);
			~Bureaucrat(void);

			Bureaucrat& operator=(Bureaucrat const& copy);

			std::string const& getName(void) const;
			int getGrade(void) const;

			void gradeUp(void);
			void gradeDown(void);

			//Exceptions
			class GradeTooHighException : public std::exception
			{
				public:
					virtual char const *what(void) const throw();
			};

			class GradeTooLowException : public std::exception
			{
				public:
					virtual char const *what(void) const throw();
			};
};

std::ostream &operator<<(std::ostream &str, Bureaucrat const& bureaucrat);

#endif
