/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 15:02:56 by mmarinov          #+#    #+#             */
/*   Updated: 2026/03/08 15:04:14 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERN_HPP
# define INTERN_HPP

# include "AForm.hpp"
# include <string>

class Intern
{
	public:
		Intern(void);
		Intern(Intern const& other);
		~Intern(void);
		Intern& operator=(Intern const& other);
		AForm* makeForm(std::string const& formName, std::string const& target) const;
};

#endif
