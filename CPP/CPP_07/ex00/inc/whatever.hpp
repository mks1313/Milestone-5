/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   whatever.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 18:36:29 by mmarinov          #+#    #+#             */
/*   Updated: 2026/05/11 20:03:58 by mmarinov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WHATEVER_HPP
# define WHATEVER_HPP

template <typename T>
void swap(T &num1, T & num2) {
	T tmp = num1;
	num1 = num2;
	num2 = tmp;
}

template <typename T>
T min(T &num1, T &num2) {
	return (num1 < num2 ? num1 : num2);
}

template <typename T>
T max(T &num1, T &num2) {
	return (num1 > num2 ? num1 : num2);
}

#endif
