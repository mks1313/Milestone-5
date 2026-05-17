/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 16:41:14 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 16:53:05 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Span.hpp"

Span::Span(unsigned int n) : _max(n) { _data.reserve(n); }

Span::Span(const Span &other) : _max(other._max), _data(other._data) {}

Span &Span::operator=(const Span &other)
{
	if (this != &other)
	{
		_max  = other._max;
		_data = other._data;
	}
	return *this;
}

Span::~Span(void) {}

void	Span::addNumber(int n)
{
	if (_data.size() >= _max)
		throw std::runtime_error("Span is full");
	_data.push_back(n);
}

int		Span::shortestSpan(void) const
{
	if (_data.size() < 2)
		throw std::runtime_error("Not enough elements to compute a span");

	std::vector<int> sorted(_data);
	std::sort(sorted.begin(), sorted.end());

	int shortest = sorted[1] - sorted[0];
	for (size_t i = 2; i < sorted.size(); ++i)
	{
		int diff = sorted[i] - sorted[i - 1];
		if (diff < shortest)
			shortest = diff;
	}
	return shortest;
}

int		Span::longestSpan(void) const
{
	if (_data.size() < 2)
		throw std::runtime_error("Not enough elements to compute a span");

	int minValue = *std::min_element(_data.begin(), _data.end());
	int maxValue = *std::max_element(_data.begin(), _data.end());
	return maxValue - minValue;
}
