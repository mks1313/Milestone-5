/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScalarConverter.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:48:51 by mmarinov          #+#    #+#             */
/*   Updated: 2026/04/16 21:26:49 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

#ifndef SCALARCONVERTER_HPP
# define SCALARCONVERTER_HPP

class ScalarConverter
{
	private:
		static bool isPseudoLiteral(const std::string& literal);
		static bool isCharLiteral(const std::string& literal);
		static bool isIntLiteral(const std::string& literal);
		static bool isFloatLiteral(const std::string& literal);
		static bool isDoubleLiteral(const std::string& literal);

		static void prinChar(double literal);
		static void printInt(double literal);
		static void printFloat(double literal);
		static void printDouble(double literaal);

	public:
		static void convert(const std::string& literal);
};

# endif
