/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 16:37:54 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 16:51:47 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAN_HPP
# define SPAN_HPP

# include <vector>
# include <stdexcept>
# include <algorithm>
# include <iterator>

class Span
{
	private:
		unsigned int		_max;
		std::vector<int>	_data;

		Span(void);

	public:
		Span(unsigned int n);
		Span(const Span &other);
		Span &operator=(const Span &other);
		~Span();

		void			addNumber(int n);
		int				shortestSpan(void) const;
		int				longestSpan(void) const;

		template <typename Iterator>
			void	addRange(Iterator first, Iterator last)
			{
				if (_data.size() + std::distance(first, last) > _max)
					throw std::runtime_error("Not enough space in Span");
				_data.insert(_data.end(), first, last);
			}
};

#endif
