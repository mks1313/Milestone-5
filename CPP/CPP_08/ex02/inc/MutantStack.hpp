/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mutantstack.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:03:38 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 17:06:49 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MUTANTSTACK_HPP
# define MUTANTSTACK_HPP

# include <stack>
# include <deque>

template <typename T, typename Container = std::deque<T> >
class MutantStack : public std::stack<T, Container>
{
public:
	MutantStack(void) {}
	MutantStack(const MutantStack &other) : std::stack<T, Container>(other) {}
	MutantStack &operator=(const MutantStack &other)
	{
		if (this != &other)
			std::stack<T, Container>::operator=(other);
		return *this;
	}
	~MutantStack(void) {}

	typedef typename Container::iterator		iterator;
	typedef typename Container::const_iterator	const_iterator;

	iterator		begin(void)       { return this->c.begin(); }
	iterator		end(void)         { return this->c.end(); }
	const_iterator	begin(void) const { return this->c.begin(); }
	const_iterator	end(void)   const { return this->c.end(); }
};

#endif
