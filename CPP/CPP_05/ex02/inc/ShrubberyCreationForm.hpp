/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShrubberyCreationForm.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 12:19:19 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 12:29:10 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHRUBBERYCREATIONFORM_HPP
# define SHRUBBERYCREATIONFORM_HPP

# include "AForm.hpp"
# include <string>

class ShrubberyCreationForm : public AForm
{
	private:
		std::string _target;

	public:
		ShrubberyCreationForm(std::string const& target);
		ShrubberyCreationForm(ShrubberyCreationForm const& other);
		~ShrubberyCreationForm(void);

		ShrubberyCreationForm& operator=(ShrubberyCreationForm const& other);

		virtual void execute(Bureaucrat const& executor) const;
};

#endif
