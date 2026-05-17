/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:29:52 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:29:55 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PMERGEME_HPP
# define PMERGEME_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include <list>
# include <ctime>
# include <climits>
# include <cstdlib>
# include <stdexcept>
# include <algorithm>
# include "Colors.hpp"

class PmergeMe
{
	private:
		std::vector<int>	_vec;
		std::list<int>		_lst;
		double				_timeVec;
		double				_timeLst;

		PmergeMe(void);

		int		parseToken(const std::string &token) const;
		bool	isSortedVector(void) const;
		bool	isSortedList(void) const;

		void	sortVector(void);
		void	sortList(void);

		void	printBefore(char **argv, int argc) const;
		void	printAfter(void) const;
		void	printTime(void) const;

	public:
		PmergeMe(int argc, char **argv);
		PmergeMe(const PmergeMe &other);
		PmergeMe &operator=(const PmergeMe &other);
		~PmergeMe(void);
};

#endif
