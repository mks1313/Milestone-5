/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:07:21 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 17:14:03 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <list>
#include <stack>

#include "MutantStack.hpp"
#include "Colors.hpp"

static void	printTitle(const std::string &title)
{
	std::cout << BOLD << MAGENTA
			  << "\n=== " << title << " ==="
			  << RESET << std::endl;
}

static void	printInfo(const std::string &label, int value)
{
	std::cout << CYAN << label << RESET
			  << ": "
			  << GREEN << value << RESET
			  << std::endl;
}

static void	testBasic(void)
{
	printTitle("TEST 1: basic push/pop/top/size");

	MutantStack<int> ms;

	ms.push(10);
	ms.push(20);
	ms.push(30);

	printInfo("top", ms.top());
	printInfo("size", ms.size());

	ms.pop();

	printInfo("top after pop", ms.top());
}

static void	testForwardIteration(void)
{
	printTitle("TEST 2: forward iteration");

	MutantStack<int> ms;

	ms.push(1);
	ms.push(2);
	ms.push(3);
	ms.push(4);
	ms.push(5);

	MutantStack<int>::iterator it = ms.begin();
	MutantStack<int>::iterator ite = ms.end();

	std::cout << YELLOW << "content: " << RESET;

	while (it != ite)
	{
		std::cout << GREEN << *it << RESET << " ";
		++it;
	}
	std::cout << std::endl;
}

static void	testIteratorArithmetic(void)
{
	printTitle("TEST 3: iterator arithmetic");

	MutantStack<int> ms;

	ms.push(10);
	ms.push(20);
	ms.push(30);

	MutantStack<int>::iterator it = ms.begin();

	printInfo("begin", *it);

	++it;
	printInfo("++it", *it);

	it++;
	printInfo("it++", *it);

	--it;
	printInfo("--it", *it);

	it--;
	printInfo("it--", *it);
}

static void	testConstIterator(void)
{
	printTitle("TEST 4: const iterator");

	MutantStack<int> ms;

	ms.push(7);
	ms.push(14);
	ms.push(21);

	const MutantStack<int> &cms = ms;

	MutantStack<int>::const_iterator it = cms.begin();
	MutantStack<int>::const_iterator ite = cms.end();

	std::cout << YELLOW << "const content: " << RESET;

	while (it != ite)
	{
		std::cout << BLUE << *it << RESET << " ";
		++it;
	}
	std::cout << std::endl;
}

static void	testCanonical(void)
{
	printTitle("TEST 5: canonical form");

	MutantStack<int> a;

	a.push(1);
	a.push(2);
	a.push(3);

	MutantStack<int> b(a);

	printInfo("copy top", b.top());
	printInfo("copy size", b.size());

	MutantStack<int> c;

	c = a;

	printInfo("assign top", c.top());
	printInfo("assign size", c.size());

	b.pop();

	printInfo("a.size after b.pop()", a.size());
}

static void	testStackConversion(void)
{
	printTitle("TEST 6: conversion to std::stack");

	MutantStack<int> ms;

	ms.push(100);
	ms.push(200);
	ms.push(300);

	std::stack<int> s(ms);

	printInfo("std::stack top", s.top());
	printInfo("std::stack size", s.size());
}

static void	testSameAsList(void)
{
	printTitle("TEST 7: same output as std::list");

	MutantStack<int> ms;

	ms.push(5);
	ms.push(17);
	ms.push(3);
	ms.push(5);
	ms.push(737);
	ms.push(0);

	std::cout << YELLOW << "MutantStack: " << RESET;

	MutantStack<int>::iterator it = ms.begin();
	MutantStack<int>::iterator ite = ms.end();

	while (it != ite)
	{
		std::cout << GREEN << *it << RESET << " ";
		++it;
	}
	std::cout << std::endl;

	std::list<int> lst;

	lst.push_back(5);
	lst.push_back(17);
	lst.push_back(3);
	lst.push_back(5);
	lst.push_back(737);
	lst.push_back(0);

	std::cout << YELLOW << "std::list:   " << RESET;

	std::list<int>::iterator lit = lst.begin();
	std::list<int>::iterator lite = lst.end();

	while (lit != lite)
	{
		std::cout << BLUE << *lit << RESET << " ";
		++lit;
	}
	std::cout << std::endl;
}

int	main(void)
{
	std::cout << BOLD << YELLOW
			  << "\n========== CPP08 / EX02 - MUTANTSTACK =========="
			  << RESET << std::endl;

	testBasic();
	testForwardIteration();
	testIteratorArithmetic();
	testConstIterator();
	testCanonical();
	testStackConversion();
	testSameAsList();

	std::cout << BOLD << GREEN
			  << "\n========== TESTS FINISHED ==========\n"
			  << RESET << std::endl;

	return (0);
}
