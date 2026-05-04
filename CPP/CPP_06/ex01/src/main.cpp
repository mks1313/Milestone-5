/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:51:09 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/04 21:19:33 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Serializer.hpp"
#include "Colors.hpp"
#include <iostream>

int main()
{
	Data* original = new Data();
	original->id = 42;
	original->name = "Hello World";
	original->value = 3.14f;

	std::cout << CYAN <<  "--------- BEFORE --------" << RESET << std::endl;
	std::cout << BLUE << "ptr:   " << RESET << RED << original << RESET << std::endl;
	std::cout << BLUE << "id:    " << RESET << RED << original->id << RESET << std::endl;
	std::cout << BLUE << "name:  " << RESET << RED << original->name << RESET << std::endl;
	std::cout << BLUE << "value: " << RESET << RED << original->value << RESET << std::endl;

	uintptr_t raw = Serializer::serialize(original);
	Data* recovered = Serializer::deserialize(raw);

	std::cout << YELLOW << "--------- AFTER ---------" << RESET << std::endl;
	std::cout << BLUE "ptr:   " << RESET << RED << original        << RESET << std::endl;
	std::cout << BLUE "id:    " << RESET << RED << original->id    << RESET << std::endl;
	std::cout << BLUE "name:  " << RESET << RED << original->name  << RESET << std::endl;
	std::cout << BLUE "value: " << RESET << RED << original->value << RESET << std::endl;

	std::cout << MAGENTA <<"\nPointer match: " <<  RESET 
		<< (original == recovered ? GREEN "YES" RESET : RED "NO" RESET) << std::endl;
	delete original;
	return 0;
}
