/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:30:16 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:47:51 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"

PmergeMe::PmergeMe(void) : _timeVec(0), _timeLst(0)
{
}

PmergeMe::PmergeMe(const PmergeMe &other)
{
	*this = other;
}

PmergeMe &PmergeMe::operator=(const PmergeMe &other)
{
	if (this != &other)
	{
		_vec = other._vec;
		_lst = other._lst;
		_timeVec = other._timeVec;
		_timeLst = other._timeLst;
	}
	return (*this);
}

PmergeMe::~PmergeMe(void)
{
}

int	PmergeMe::parseToken(const std::string &token) const
{
	char	*end;
	long	value;

	if (token.empty())
		throw std::runtime_error("Error");

	value = std::strtol(token.c_str(), &end, 10);
	if (*end != '\0')
		throw std::runtime_error("Error");
	if (value < 0 || value > INT_MAX)
		throw std::runtime_error("Error");

	return (static_cast<int>(value));
}

PmergeMe::PmergeMe(int argc, char **argv) : _timeVec(0), _timeLst(0)
{
	int		i;
	int		value;
	clock_t	start;
	clock_t	end;

	i = 1;
	while (i < argc)
	{
		value = parseToken(argv[i]);
		_vec.push_back(value);
		_lst.push_back(value);
		i++;
	}

	printBefore(argv, argc);

	start = clock();
	sortVector();
	end = clock();
	_timeVec = static_cast<double>(end - start) / CLOCKS_PER_SEC;

	start = clock();
	sortList();
	end = clock();
	_timeLst = static_cast<double>(end - start) / CLOCKS_PER_SEC;

	printAfter();
	printTime();
}

void	PmergeMe::sortVector(void)
{
	std::vector<int>	mainChain;
	std::vector<int>	pending;
	std::vector<int>::iterator	it;
	size_t				i;

	i = 0;
	while (i + 1 < _vec.size())
	{
		if (_vec[i] < _vec[i + 1])
		{
			pending.push_back(_vec[i]);
			mainChain.push_back(_vec[i + 1]);
		}
		else
		{
			pending.push_back(_vec[i + 1]);
			mainChain.push_back(_vec[i]);
		}
		i += 2;
	}

	if (i < _vec.size())
		pending.push_back(_vec[i]);

	std::sort(mainChain.begin(), mainChain.end());

	i = 0;
	while (i < pending.size())
	{
		it = std::lower_bound(mainChain.begin(), mainChain.end(), pending[i]);
		mainChain.insert(it, pending[i]);
		i++;
	}

	_vec = mainChain;
}

void	PmergeMe::sortList(void)
{
	std::list<int>	mainChain;
	std::list<int>	pending;
	std::list<int>::iterator	it;
	std::list<int>::iterator	next;

	it = _lst.begin();
	while (it != _lst.end())
	{
		next = it;
		++next;

		if (next == _lst.end())
		{
			pending.push_back(*it);
			break ;
		}

		if (*it < *next)
		{
			pending.push_back(*it);
			mainChain.push_back(*next);
		}
		else
		{
			pending.push_back(*next);
			mainChain.push_back(*it);
		}

		it = next;
		++it;
	}

	mainChain.sort();

	it = pending.begin();
	while (it != pending.end())
	{
		std::list<int>::iterator pos = mainChain.begin();
		while (pos != mainChain.end() && *pos < *it)
			++pos;
		mainChain.insert(pos, *it);
		++it;
	}

	_lst = mainChain;
}

void	PmergeMe::printBefore(char **argv, int argc) const
{
	int	i;

	std::cout << BOLD << CYAN << "Before: " << RESET;
	i = 1;
	while (i < argc)
	{
		std::cout << argv[i];
		if (i + 1 < argc)
			std::cout << " ";
		i++;
	}
	std::cout << std::endl;
}

void	PmergeMe::printAfter(void) const
{
	std::vector<int>::const_iterator	it;

	std::cout << BOLD << GREEN << "After:  " << RESET;
	it = _vec.begin();
	while (it != _vec.end())
	{
		std::cout << *it;
		++it;
		if (it != _vec.end())
			std::cout << " ";
	}
	std::cout << std::endl;
}

void	PmergeMe::printTime(void) const
{
	std::cout << YELLOW
			  << "Time to process a range of "
			  << _vec.size()
			  << " elements with std::vector : "
			  << RESET
			  << _timeVec
			  << " us"
			  << std::endl;

	std::cout << YELLOW
			  << "Time to process a range of "
			  << _lst.size()
			  << " elements with std::list   : "
			  << RESET
			  << _timeLst
			  << " us"
			  << std::endl;
}
