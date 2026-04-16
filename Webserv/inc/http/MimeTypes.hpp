/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/11 16:02:02 by vberdugo          #+#    #+#             */
/*   Updated: 2026/04/10 20:10:40 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIMETYPES_HPP
# define MIMETYPES_HPP

# include <string>
# include <map>

class MimeTypes {
public:
	// Singleton access
	static MimeTypes&				getInstance();

	// MIME type lookup
	std::string						getMimeType(const std::string& extension) const;
	std::string						getMimeTypeByFile(const std::string& filename) const;
	bool							isTextType(const std::string& mimeType) const;
	bool							isBinaryType(const std::string& mimeType) const;

	// Custom types
	void							addMimeType(const std::string& extension, const std::string& mimeType);

private:
	MimeTypes();
	~MimeTypes();
	MimeTypes(const MimeTypes&);
	MimeTypes& operator=(const MimeTypes&);

	std::map<std::string, std::string>	_mimeTypes;

	void							_initMimeTypes();
};

#endif
