/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScalarConverter.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:48:51 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/03 16:31:09 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <climits>
#include <cstdlib>
#include <cmath>

#ifndef SCALARCONVERTER_HPP
# define SCALARCONVERTER_HPP

typedef enum e_type
{
	TYPE_CHAR,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_PSEUDO_LITERAL,
	TYPE_INVALID
} t_type;

class ScalarConverter
{
	private:
		static t_type detectType(const std::string& literal);
		static bool isPseudoLiteral(const std::string& literal);
		static bool isCharLiteral(const std::string& literal);
		static bool isIntLiteral(const std::string& literal);
		static bool isFloatLiteral(const std::string& literal);
		static bool isDoubleLiteral(const std::string& literal);

		static void printChar(double value, t_type type, const std::string& literal);
		static void printInt(double value, t_type type, const std::string& literal);
		static void printFloat(double value, t_type type, const std::string& literal);
		static void printDouble(double value, t_type type, const std::string& literal);

	public:
		static void convert(const std::string& literal);
};

# endif
