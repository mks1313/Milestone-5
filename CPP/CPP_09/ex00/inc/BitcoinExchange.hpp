/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:06:10 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:08:50 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITCOINEXCHANGE_HPP
# define BITCOINEXCHANGE_HPP

# include <map>
# include <string>
# include <iostream>
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <cstdlib>
# include <iomanip>
# include <limits>
# include "Colors.hpp"

class BitcoinExchange
{
	private:
		std::map<std::string, double> _data;

		void		readCSV(const std::string &filename);
		bool		isValidDate(const std::string &date) const;
		bool		isLeapYear(int year) const;
		bool		parseDouble(const std::string &str, double &value) const;
		std::string	trim(const std::string &str) const;
		double		getRateForDate(const std::string &date) const;

	public:
		BitcoinExchange(void);
		BitcoinExchange(const BitcoinExchange &other);
		BitcoinExchange &operator=(const BitcoinExchange &other);
		~BitcoinExchange(void);

		void		processFile(const std::string &filename) const;
};

#endif
