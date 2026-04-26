/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScalarConverter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 19:34:15 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/16 19:34:50 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ScalarConverter.hpp"
#include <cctype>

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

bool ScalarConverter::isFloatLiteral(const std::string& literal)
{
	const std::string& str = literal;
	return false;
}

bool ScalarConverter::isDoubleLiteral(const std::string& literal)
{
	const std::string& str = literal;
	return false;
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
