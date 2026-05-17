/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:30:25 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:30:45 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"
#include "Colors.hpp"

#include <iostream>
#include <cstdlib>

static void	printHeader(void)
{
	std::cout << BOLD << MAGENTA
			  << "\n========== CPP09 / EX02 - PMERGEME =========="
			  << RESET << std::endl;
}

int	main(int argc, char **argv)
{
	try
	{
		if (argc < 2)
			throw std::runtime_error("Error");

		printHeader();
		PmergeMe sorter(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << BOLD << RED
				  << e.what()
				  << RESET << std::endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
