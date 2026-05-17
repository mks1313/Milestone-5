/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 11:42:04 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 14:00:58 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <string>

#include "Easyfind.hpp"
#include "Colors.hpp"

template <typename T>
void	test(T &container, int value, const std::string &label)
{
	std::cout << YELLOW << label << RESET
			  << " -> buscar "
			  << BLUE << value << RESET
			  << ": ";
	try
	{
		typename T::iterator it = easyfind(container, value);
		std::cout << GREEN << "encontrado"
				  << RESET << " (valor=" << *it << ")" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << RED << "no encontrado"
				  << RESET << " [" << e.what() << "]" << std::endl;
	}
}

int	main(void)
{
	std::cout << BOLD << CYAN
			  << "========== CPP08 / EX00 - EASYFIND =========="
			  << RESET << std::endl;

	std::cout << BOLD << MAGENTA
			  << "\n=== std::vector ==="
			  << RESET << std::endl;

	std::vector<int> vec;
	vec.push_back(10);
	vec.push_back(20);
	vec.push_back(30);
	vec.push_back(42);
	vec.push_back(-5);

	test(vec, 42, "vector");
	test(vec, -5, "vector");
	test(vec, 10, "vector");
	test(vec, 99, "vector");

	std::cout << BOLD << MAGENTA
			  << "\n=== std::list ==="
			  << RESET << std::endl;

	std::list<int> lst;
	lst.push_back(1);
	lst.push_back(2);
	lst.push_back(3);

	test(lst, 2, "list");
	test(lst, 0, "list");

	std::cout << BOLD << MAGENTA
			  << "\n=== std::deque ==="
			  << RESET << std::endl;

	std::deque<int> dq;
	dq.push_back(100);
	dq.push_back(200);
	dq.push_back(300);

	test(dq, 200, "deque");
	test(dq, 999, "deque");

	std::cout << BOLD << MAGENTA
			  << "\n=== contenedor vacio ==="
			  << RESET << std::endl;

	std::vector<int> empty;
	test(empty, 0, "vector vacio");

	std::cout << BOLD << CYAN
			  << "\n========== TESTS FINISHED =========="
			  << RESET << std::endl;

	return (0);
}
