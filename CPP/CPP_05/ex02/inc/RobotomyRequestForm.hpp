/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RobotomyRequestForm.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:32:25 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:05:45 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROBOTOMYREQUESTFORM_HPP
# define ROBOTOMYREQUESTFORM_HPP

# include "AForm.hpp"
# include <string>

class RobotomyRequestForm : public AForm
{
	private:
		std::string _target;

	public:
		RobotomyRequestForm(std::string const& target);
		RobotomyRequestForm(RobotomyRequestForm const& other);
		~RobotomyRequestForm(void);

		RobotomyRequestForm& operator=(RobotomyRequestForm const& other);

		virtual void execute(Bureaucrat const& executor) const;
};

#endif
