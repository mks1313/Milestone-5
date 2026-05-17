/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 18:07:05 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/17 18:07:11 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"

BitcoinExchange::BitcoinExchange(void)
{
	readCSV("data.csv");
}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other)
{
	*this = other;
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other)
{
	if (this != &other)
		_data = other._data;
	return (*this);
}

BitcoinExchange::~BitcoinExchange(void)
{
}

std::string	BitcoinExchange::trim(const std::string &str) const
{
	size_t	start;
	size_t	end;

	start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return ("");
	end = str.find_last_not_of(" \t\r\n");
	return (str.substr(start, end - start + 1));
}

bool	BitcoinExchange::isLeapYear(int year) const
{
	if (year % 400 == 0)
		return (true);
	if (year % 100 == 0)
		return (false);
	if (year % 4 == 0)
		return (true);
	return (false);
}

bool	BitcoinExchange::isValidDate(const std::string &date) const
{
	int			year;
	int			month;
	int			day;
	int			daysInMonth[12];
	char		dash1;
	char		dash2;
	std::istringstream	ss(date);

	if (date.length() != 10)
		return (false);
	if (date[4] != '-' || date[7] != '-')
		return (false);
	for (size_t i = 0; i < date.length(); ++i)
	{
		if (i != 4 && i != 7 && !std::isdigit(date[i]))
			return (false);
	}

	ss >> year >> dash1 >> month >> dash2 >> day;
	if (ss.fail() || dash1 != '-' || dash2 != '-')
		return (false);
	if (month < 1 || month > 12)
		return (false);

	daysInMonth[0] = 31;
	daysInMonth[1] = 28;
	daysInMonth[2] = 31;
	daysInMonth[3] = 30;
	daysInMonth[4] = 31;
	daysInMonth[5] = 30;
	daysInMonth[6] = 31;
	daysInMonth[7] = 31;
	daysInMonth[8] = 30;
	daysInMonth[9] = 31;
	daysInMonth[10] = 30;
	daysInMonth[11] = 31;

	if (isLeapYear(year))
		daysInMonth[1] = 29;
	if (day < 1 || day > daysInMonth[month - 1])
		return (false);
	return (true);
}

bool	BitcoinExchange::parseDouble(const std::string &str, double &value) const
{
	std::istringstream	ss(str);
	char				leftover;

	ss >> value;
	if (ss.fail())
		return (false);
	if (ss >> leftover)
		return (false);
	return (true);
}

void	BitcoinExchange::readCSV(const std::string &filename)
{
	std::ifstream	file(filename.c_str());
	std::string		line;
	std::string		date;
	std::string		valueStr;
	double			value;

	if (!file)
		throw std::runtime_error("could not open data.csv");

	if (std::getline(file, line))
	{
		if (line != "date,exchange_rate")
		{
			std::istringstream	first(line);
			std::getline(first, date, ',');
			std::getline(first, valueStr);
			date = trim(date);
			valueStr = trim(valueStr);
			if (isValidDate(date) && parseDouble(valueStr, value))
				_data[date] = value;
		}
	}

	while (std::getline(file, line))
	{
		std::istringstream	iss(line);

		std::getline(iss, date, ',');
		std::getline(iss, valueStr);

		date = trim(date);
		valueStr = trim(valueStr);

		if (!isValidDate(date))
			continue;
		if (!parseDouble(valueStr, value))
			continue;
		_data[date] = value;
	}

	if (_data.empty())
		throw std::runtime_error("data.csv is empty or invalid");
}

double	BitcoinExchange::getRateForDate(const std::string &date) const
{
	std::map<std::string, double>::const_iterator	it;

	it = _data.lower_bound(date);
	if (it != _data.end() && it->first == date)
		return (it->second);
	if (it == _data.begin())
		throw std::runtime_error("no earlier date available");
	--it;
	return (it->second);
}

void	BitcoinExchange::processFile(const std::string &filename) const
{
	std::ifstream	file(filename.c_str());
	std::string		line;
	std::string		date;
	std::string		valueStr;
	double			value;
	double			rate;

	if (!file)
		throw std::runtime_error("could not open file");

	if (std::getline(file, line))
	{
		if (trim(line) != "date | value")
		{
			std::cerr << ERROR << "Error: bad header" << RESET << std::endl;
			return ;
		}
	}

	while (std::getline(file, line))
	{
		std::istringstream	iss(line);

		if (line.empty())
			continue;

		if (!std::getline(iss, date, '|') || !std::getline(iss, valueStr))
		{
			std::cerr << ERROR << "Error: bad input => "
					  << line << RESET << std::endl;
			continue;
		}

		date = trim(date);
		valueStr = trim(valueStr);

		if (!isValidDate(date))
		{
			std::cerr << ERROR << "Error: bad input => "
					  << date << RESET << std::endl;
			continue;
		}

		if (!parseDouble(valueStr, value))
		{
			std::cerr << ERROR << "Error: bad input => "
					  << line << RESET << std::endl;
			continue;
		}

		if (value < 0)
		{
			std::cerr << ERROR << "Error: not a positive number."
					  << RESET << std::endl;
			continue;
		}

		if (value > 1000)
		{
			std::cerr << ERROR << "Error: too large a number."
					  << RESET << std::endl;
			continue;
		}

		try
		{
			rate = getRateForDate(date);
			std::cout << date << " => " << value
					  << " = " << value * rate << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cerr << ERROR << "Error: " << e.what()
					  << RESET << std::endl;
		}
	}
}
