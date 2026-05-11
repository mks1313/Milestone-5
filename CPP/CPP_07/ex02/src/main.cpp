/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 20:24:19 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/11 21:18:24 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Array.hpp"
#include "Colors.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

template <typename T>
void printArray(Array<T> const &arr, std::string const &name)
{
	std::cout << BOLD << CYAN << name << RESET
			  << " size = " << GREEN << arr.size() << RESET << std::endl;

	for (unsigned int i = 0; i < arr.size(); i++)
		std::cout << "  [" << i << "] = " << YELLOW << arr[i] << RESET << std::endl;
}

void testIntArray(void)
{
	std::cout << BOLD << MAGENTA << "\n--- INT ARRAY TEST ---\n" << RESET;

	Array<int> nums(5);

	for (unsigned int i = 0; i < nums.size(); i++)
		nums[i] = static_cast<int>(i * 10);

	printArray(nums, "nums");
}

void testStringArray(void)
{
	std::cout << BOLD << MAGENTA << "\n--- STRING ARRAY TEST ---\n" << RESET;

	Array<std::string> words(3);

	words[0] = "hello";
	words[1] = "cpp";
	words[2] = "templates";

	printArray(words, "words");
}

void testDeepCopy(void)
{
	std::cout << BOLD << MAGENTA << "\n--- DEEP COPY TEST ---\n" << RESET;

	Array<int> original(3);

	original[0] = 1;
	original[1] = 2;
	original[2] = 3;

	Array<int> copy(original);

	copy[0] = 999;

	printArray(original, "original");
	printArray(copy, "copy");

	std::cout << GREEN
			  << "If original[0] is still 1, deep copy works."
			  << RESET << std::endl;
}

void testAssignment(void)
{
	std::cout << BOLD << MAGENTA << "\n--- ASSIGNMENT TEST ---\n" << RESET;

	Array<int> a(2);
	Array<int> b(4);

	a[0] = 10;
	a[1] = 20;

	b[0] = 1;
	b[1] = 2;
	b[2] = 3;
	b[3] = 4;

	b = a;
	b[0] = 777;

	printArray(a, "a");
	printArray(b, "b");

	std::cout << GREEN
			  << "If a[0] is still 10, assignment deep copy works."
			  << RESET << std::endl;
}

void testException(void)
{
	std::cout << BOLD << MAGENTA << "\n--- EXCEPTION TEST ---\n" << RESET;

	Array<int> nums(2);

	try
	{
		std::cout << "Trying nums[10]..." << std::endl;
		nums[10] = 42;
	}
	catch (std::exception const &e)
	{
		std::cout << BOLD << RED
				  << "Caught exception: "
				  << RESET << e.what() << std::endl;
	}
}

void testEmptyArray(void)
{
	std::cout << BOLD << MAGENTA << "\n--- EMPTY ARRAY TEST ---\n" << RESET;

	Array<int> empty;

	std::cout << "empty size = "
			  << GREEN << empty.size() << RESET << std::endl;

	try
	{
		std::cout << "Trying empty[0]..." << std::endl;
		std::cout << empty[0] << std::endl;
	}
	catch (std::exception const &e)
	{
		std::cout << BOLD << RED
				  << "Caught exception: "
				  << RESET << e.what() << std::endl;
	}
}

int main(void)
{
	std::cout << BOLD << YELLOW
			  << "\n========== ARRAY TEMPLATE TESTER ==========\n"
			  << RESET;

	testIntArray();
	testStringArray();
	testDeepCopy();
	testAssignment();
	testException();
	testEmptyArray();

	std::cout << BOLD << GREEN
			  << "\nAll tests finished.\n"
			  << RESET;

	return (0);
}
