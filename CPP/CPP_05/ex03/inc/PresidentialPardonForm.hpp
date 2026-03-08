/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PresidentialPardonForm.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:53:23 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 14:59:23 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PRESIDENTIALPARDONFORM_HPP
# define PRESIDENTIALPARDONFORM_HPP

#include "AForm.hpp"
# include <string>

class PresidentialPardonForm : public AForm
{
	private:
		std::string _target;

	public:
		PresidentialPardonForm(std::string const& target);
		PresidentialPardonForm(PresidentialPardonForm const& other);
		~PresidentialPardonForm(void);

		PresidentialPardonForm& operator=(PresidentialPardonForm const& other);

		virtual void execute(Bureaucrat const& executor) const;
};

#endif
