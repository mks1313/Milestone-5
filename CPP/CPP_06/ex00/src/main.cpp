/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 16:24:17 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/03 16:56:13 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ScalarConverter.hpp"
#include "Colors.hpp"
#include <iostream>

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cout << RED << BOLD << "Error: " << RESET << "usage: ./converter <literal>" << std::endl;
		return 1;
	}
	std::string literal = av[1];
	ScalarConverter::convert(literal);
	return 0;
}
