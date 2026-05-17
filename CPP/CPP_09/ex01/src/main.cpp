/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:23:49 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:23:53 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RPN.hpp"
#include "Colors.hpp"

#include <iostream>
#include <string>

static void	printStringInCenter(const std::string &str)
{
	const int	width = 60;
	int			padding;
	int			i;

	padding = (width - static_cast<int>(str.length())) / 2;
	if (padding < 0)
		padding = 0;

	std::cout << BOLD << CYAN;
	i = 0;
	while (i < padding)
	{
		std::cout << " ";
		i++;
	}
	std::cout << str << RESET << std::endl;
}

int	main(int argc, char **argv)
{
	try
	{
		if (argc != 2)
			throw std::runtime_error("Error: usage: ./RPN \"8 9 * 9 - 9 - 9 - 4 - 1 +\"");

		printStringInCenter(argv[1]);
		RPN calc(argv[1]);
	}
	catch (const std::exception &e)
	{
		std::cerr << BOLD << RED
				  << e.what()
				  << RESET << std::endl;
		return (1);
	}

	return (0);
}
