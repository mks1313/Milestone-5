/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 20:06:15 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/11 20:15:39 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "iter.hpp"
#include "Colors.hpp"

#include <iostream>

#include "iter.hpp"
#include "Colors.hpp"

#include <iostream>
#include <string>

template <typename T>
void printValue(T const &value)
{
	std::cout << GREEN << value << RESET << std::endl;
}

template <typename T>
void printBox(T const &value)
{
	std::cout << CYAN << "[ " << RESET
			  << value
			  << CYAN << " ]" << RESET << std::endl;
}

void incrementInt(int &value)
{
	value++;
}

void doubleFloat(float &value)
{
	value *= 2;
}

void shoutString(std::string &value)
{
	value += "!";
}

int main(void)
{
	int nums[] = {1, 2, 3, 4, 5};
	float floats[] = {1.5f, 2.5f, 3.5f};
	std::string words[] = {"hello", "cpp", "templates"};
	const int constNums[] = {10, 20, 30};

	std::cout << BOLD << YELLOW
			  << "\n========== ITER TEMPLATE TESTER ==========\n"
			  << RESET << std::endl;

	std::cout << BOLD << MAGENTA << "\n--- int array: print ---\n" << RESET;
	iter(nums, 5, printValue<int>);

	std::cout << BOLD << MAGENTA << "\n--- int array: increment ---\n" << RESET;
	iter(nums, 5, incrementInt);
	iter(nums, 5, printBox<int>);

	std::cout << BOLD << MAGENTA << "\n--- float array: print ---\n" << RESET;
	iter(floats, 3, printValue<float>);

	std::cout << BOLD << MAGENTA << "\n--- float array: double ---\n" << RESET;
	iter(floats, 3, doubleFloat);
	iter(floats, 3, printBox<float>);

	std::cout << BOLD << MAGENTA << "\n--- string array: print ---\n" << RESET;
	iter(words, 3, printValue<std::string>);

	std::cout << BOLD << MAGENTA << "\n--- string array: shout ---\n" << RESET;
	iter(words, 3, shoutString);
	iter(words, 3, printBox<std::string>);

	std::cout << BOLD << MAGENTA << "\n--- const int array: print only ---\n" << RESET;
	iter(constNums, 3, printValue<int>);

	std::cout << BOLD << GREEN
			  << "\nAll tests finished.\n"
			  << RESET;

	return (0);
}
