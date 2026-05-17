/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:23:03 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:23:07 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RPN_HPP
# define RPN_HPP

# include <stack>
# include <string>
# include <sstream>
# include <iostream>
# include <stdexcept>
# include <cstdlib>

class RPN
{
	private:
		std::stack<int>	_stack;

		RPN(void);

		bool	isOperator(const std::string &token) const;
		bool	isNumberToken(const std::string &token) const;
		int		applyOperator(int left, int right, const std::string &op) const;

	public:
		RPN(const std::string &expression);
		RPN(const RPN &other);
		RPN &operator=(const RPN &other);
		~RPN(void);
};

#endif
