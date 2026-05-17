/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 16:41:42 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 16:49:42 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <list>
#include <cstdlib>

#include "Span.hpp"
#include "Colors.hpp"

static void	printTitle(const std::string &title)
{
	std::cout << BOLD << MAGENTA
			  << "\n=== " << title << " ==="
			  << RESET << std::endl;
}

static void	printResult(const std::string &label, int result)
{
	std::cout << CYAN << label << RESET
			  << ": "
			  << GREEN << result << RESET
			  << std::endl;
}

static void	printOk(const std::string &msg)
{
	std::cout << GREEN << "OK: " << RESET
			  << msg << std::endl;
}

static void	printError(const std::string &msg)
{
	std::cout << RED << "ERROR: " << RESET
			  << msg << std::endl;
}

static void	testBasic(void)
{
	printTitle("TEST 1: basic subject example");

	Span sp(5);
	sp.addNumber(6);
	sp.addNumber(3);
	sp.addNumber(17);
	sp.addNumber(9);
	sp.addNumber(11);

	printResult("shortestSpan", sp.shortestSpan());
	printResult("longestSpan", sp.longestSpan());
}

static void	testFull(void)
{
	printTitle("TEST 2: full span");

	Span sp(2);
	sp.addNumber(1);
	sp.addNumber(2);

	try
	{
		sp.addNumber(3);
		printError("exception should have been thrown");
	}
	catch (const std::exception &e)
	{
		printOk(e.what());
	}
}

static void	testTooFew(void)
{
	printTitle("TEST 3: too few elements");

	Span empty(5);
	try
	{
		empty.shortestSpan();
		printError("empty span should throw");
	}
	catch (const std::exception &e)
	{
		printOk(std::string("empty span -> ") + e.what());
	}

	Span one(5);
	one.addNumber(42);

	try
	{
		one.longestSpan();
		printError("single element span should throw");
	}
	catch (const std::exception &e)
	{
		printOk(std::string("single element -> ") + e.what());
	}
}

static void	testAddRangeVector(void)
{
	printTitle("TEST 4: addRange with vector");

	std::vector<int> v;
	v.push_back(5);
	v.push_back(15);
	v.push_back(3);
	v.push_back(10);

	Span sp(10);
	sp.addRange(v.begin(), v.end());

	printResult("shortestSpan", sp.shortestSpan());
	printResult("longestSpan", sp.longestSpan());
}

static void	testAddRangeList(void)
{
	printTitle("TEST 5: addRange with list");

	std::list<int> lst;
	lst.push_back(100);
	lst.push_back(200);
	lst.push_back(150);

	Span sp(5);
	sp.addRange(lst.begin(), lst.end());

	printResult("shortestSpan", sp.shortestSpan());
	printResult("longestSpan", sp.longestSpan());
}

static void	testLarge(void)
{
	printTitle("TEST 6: 10 000 random numbers");

	const unsigned int N = 10000;
	Span sp(N);
	std::vector<int> big;

	big.reserve(N);
	std::srand(42);
	for (unsigned int i = 0; i < N; ++i)
		big.push_back(std::rand());

	sp.addRange(big.begin(), big.end());

	printResult("shortestSpan", sp.shortestSpan());
	printResult("longestSpan", sp.longestSpan());
}

static void	testCanonical(void)
{
	printTitle("TEST 7: canonical form");

	Span a(5);
	a.addNumber(1);
	a.addNumber(10);
	a.addNumber(100);

	Span b(a);
	printResult("copy constructor longestSpan", b.longestSpan());

	Span c(3);
	c = a;
	printResult("assignment operator shortestSpan", c.shortestSpan());
}

static void	testAllSame(void)
{
	printTitle("TEST 8: all elements equal");

	Span sp(4);
	sp.addNumber(7);
	sp.addNumber(7);
	sp.addNumber(7);
	sp.addNumber(7);

	printResult("shortestSpan", sp.shortestSpan());
	printResult("longestSpan", sp.longestSpan());
}

int	main(void)
{
	std::cout << BOLD << YELLOW
			  << "\n========== CPP08 / EX01 - SPAN TESTER =========="
			  << RESET << std::endl;

	testBasic();
	testFull();
	testTooFew();
	testAddRangeVector();
	testAddRangeList();
	testLarge();
	testCanonical();
	testAllSame();

	std::cout << BOLD << GREEN
			  << "\n========== TESTS FINISHED ==========\n"
			  << RESET;

	return (0);
}

/*
int main()
{
	Span sp = Span(5);
	sp.addNumber(6);
	sp.addNumber(3);
	sp.addNumber(17);
	sp.addNumber(9);
	sp.addNumber(11);
	std::cout << sp.shortestSpan() << std::endl;
	std::cout << sp.longestSpan() << std::endl;
	retun 0;
}*/
