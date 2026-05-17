/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:23:26 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:23:29 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RPN.hpp"

RPN::RPN(void)
{
}

RPN::RPN(const RPN &other)
{
	*this = other;
}

RPN &RPN::operator=(const RPN &other)
{
	if (this != &other)
		_stack = other._stack;
	return (*this);
}

RPN::~RPN(void)
{
}

bool	RPN::isOperator(const std::string &token) const
{
	if (token == "+")
		return (true);
	if (token == "-")
		return (true);
	if (token == "*")
		return (true);
	if (token == "/")
		return (true);
	return (false);
}

bool	RPN::isNumberToken(const std::string &token) const
{
	if (token.length() != 1)
		return (false);
	if (token[0] >= '0' && token[0] <= '9')
		return (true);
	return (false);
}

int	RPN::applyOperator(int left, int right, const std::string &op) const
{
	if (op == "+")
		return (left + right);
	if (op == "-")
		return (left - right);
	if (op == "*")
		return (left * right);
	if (op == "/")
	{
		if (right == 0)
			throw std::runtime_error("Error: division by zero.");
		return (left / right);
	}
	throw std::runtime_error("Error: invalid operator.");
}

RPN::RPN(const std::string &expression)
{
	std::istringstream	iss(expression);
	std::string			token;
	int					left;
	int					right;

	while (iss >> token)
	{
		if (isNumberToken(token))
			_stack.push(std::atoi(token.c_str()));
		else if (isOperator(token))
		{
			if (_stack.size() < 2)
				throw std::runtime_error("Error: invalid RPN expression.");

			right = _stack.top();
			_stack.pop();

			left = _stack.top();
			_stack.pop();

			_stack.push(applyOperator(left, right, token));
		}
		else
			throw std::runtime_error("Error: invalid token.");
	}

	if (_stack.size() != 1)
		throw std::runtime_error("Error: invalid RPN expression.");

	std::cout << _stack.top() << std::endl;
}
