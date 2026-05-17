/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:07:36 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:15:33 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"
#include "Colors.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << BOLD << RED
				  << "Error: could not open file."
				  << RESET << std::endl;
		return (1);
	}

	try
	{
		BitcoinExchange exchange;
		exchange.processFile(argv[1]);
	}
	catch (const std::exception &e)
	{
		std::cerr << BOLD << RED
				  << "Error: " << e.what()
				  << RESET << std::endl;
		return (1);
	}

	return (0);
}
