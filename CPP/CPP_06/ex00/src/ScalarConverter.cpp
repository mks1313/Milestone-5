/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScalarConverter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 19:34:15 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/27 23:00:53 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ScalarConverter.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <climits>

bool ScalarConverter::isPseudoLiteral(const std::string& literal)
{
	const std::string& str = literal;
	if (str == "nan" || str == "nanf" || str == "+inf" \
			|| str == "-inf" || str == "+inff" || str == "-inff")
		return true;
	return false;
}

bool ScalarConverter::isCharLiteral(const std::string& literal)
{
	const std::string& str = literal;
	if (str.size() == 1 && !isdigit(static_cast<unsigned char>(str[0])))
		return true;
	return false;
}

bool ScalarConverter::isIntLiteral(const std::string& literal)
{
	size_t i = 0;
	const std::string& str = literal;
	if (str.empty())
		return  false;
	if (str[0] == '-' || str[0] == '+')
		i++;
	if (i == str.size())
		return false;

	while (i < str.size())
	{
		if (!isdigit(static_cast<unsigned char>(str[i])))
			return false;
		i++;
	}
	return true;
}

bool ScalarConverter::isDoubleLiteral(const std::string& literal)
{
	const std::string& str = literal;
	size_t i = 0;
	bool dot = false;
	bool has_digit_before_dot = false;
	bool has_digit_after_dot = false;

	if (str.empty())
		return false;
	if (str[0] == '-' || str[0] == '+')
		i++;
	if (i == str.size())
		return false;

	while (i < str.size())
	{
		if (str[i] == '.' && !dot)
			dot = true;
		else if (isdigit(static_cast<unsigned char>(str[i])))
		{
			if (!dot)
				has_digit_before_dot = true;
			else
				has_digit_after_dot = true;
		}
		else
			return false;
		i++;
	}

	return (dot && has_digit_before_dot && has_digit_after_dot);
}

bool ScalarConverter::isFloatLiteral(const std::string& literal)
{
	const std::string& str = literal;
	if (str.empty() || str[str.size() - 1] != 'f')
		return false;
	std::string without_f = str.substr(0, str.size() - 1);
	return isDoubleLiteral(without_f);
}

t_type ScalarConverter::detectType(const std::string& literal) {
	if (literal.empty())
		return TYPE_INVALID;
	if (isPseudoLiteral(literal))
		return TYPE_PSEUDO_LITERAL;
	if (isCharLiteral(literal))
		return TYPE_CHAR;
	if (isIntLiteral(literal))
		return TYPE_INT;
	if (isFloatLiteral(literal))
		return TYPE_FLOAT;
	if (isDoubleLiteral(literal))
		return TYPE_DOUBLE;
	return TYPE_INVALID;
}

void ScalarConverter::printChar(double value, t_type type, std::string& literal)
{
	(void)literal;
	if (type = TYPE_PSEUDO_LITERAL || type == TYPE_INVALID)
	{
		std::cout << "char: impossible" << std::endl;
		return ;
	}
	if (value < 0 || value > 127)
	{
		std::cout << "char: impossible" << std::endl;
		return ;
	}
	if (!isprint(static_cast<int>(value)))
	{
		std::cout << "char: impossible" << std::endl;
		return ;
	}
	std::cout << "char: '" << static_cast<int>(value) << "'" << std::endl;
}

void ScalarConverter::printInt(double value, t_type type, std::string& literal)
{
	(void)literal;
	if (type == TYPE_PSEUDO_LITERAL || type == TYPE_INVALID)
	{
		std::cout << "int: impossible" << std::endl;
		return ;
	}
	if (value < static_cast<int>(INT_MIN) || value > static_cast<int>(INT_MAX))
	{
		std::cout << "int: impossible" << std::endl;
		return ;
	}
	std::cout << "int: '" << static_cast<int>(value) << std::endl;
}

void ScalarConverter::printFloat(double value, t_type type, std::string& literal)
{
	const std::string str = literal;
	if (type == TYPE_PSEUDO_LITERAL)
	{
		if (str == "nan" || str == "nanf")
			std::cout << "float: nanf" << std::endl;
		else if (str == "+inf" || str == "+inff")
			std::cout << "float: +inff" << std::endl;
		else 
			std::cout << "float: -inff" << std::endl;
		return ;
	}
	if (type == TYPE_INVALID)
	{
		std::cout << "float: impossible" << std::endl;
		return ;
	}
	std::cout << "float: " << static_cast<float>(value) << "f" << std::endl;
}

void ScalarConverter::printDouble(double value, t_type type, std::string& literal)
{
	if (type == TYPE_PSEUDO_LITERAL)
	{
		if (str == "nan" || str == "nanf")
			std::cout << "double: nan" << std::endl;
		else if (str == "+inf" || str == "+inff")
			std::cout << "double: +inf" << std::endl;
		else
			std::cout << "double: -inf" << std::endl;
		return ;
	}
	if (type == TYPE_INVALID)
	{
		std::cout << "double: impossible" << std::endl;
		return ;
	}
	std::cout << "double: '" << value << std::endl;
}

void ScalarConverter::convert(const std::string& literal)
{
	double value;
	const std::string str = literal;
	t_type type = detectType(str);
	
	if (type == TYPE_INVALID)
	{
		std::cout << "char: impossible" << std::endl;
		std::cout << "int: impossible" << std::endl;
		std::cout << "float: impossible" << std::endl;
		std::cout << "double: impossible" << std::endl;
		return ;
	}
	if (type == TYPE_CHAR)
		value = static_cast<double>(str[0]);
	else if (type == TYPE_PSEUDO_LITERAL)
		value = 0;
	else
		value = strtod(str.c_str(), NULL);
	printChar(value, type, str);
	printInt(value, type, str);
	printFloat(value, type, str);
	printDouble(value, type, str);
}

