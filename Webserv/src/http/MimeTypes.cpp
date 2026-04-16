/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcela-ga <fcela-ga@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/11 16:02:24 by vberdugo          #+#    #+#             */
/*   Updated: 2026/04/10 20:11:40 by fcela-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/http/MimeTypes.hpp"
#include <cctype>

// Default MIME type for unknown extensions
static const char* DEFAULT_MIME_TYPE = "application/octet-stream";

// ============================================================================
// Singleton access
// ============================================================================

MimeTypes& MimeTypes::getInstance()
{
    static MimeTypes instance;
    return instance;
}

// ============================================================================
// Constructors / Destructor
// ============================================================================

MimeTypes::MimeTypes()
{
    _initMimeTypes();
}

MimeTypes::~MimeTypes()
{
}

MimeTypes::MimeTypes(const MimeTypes& other)
{
    (void)other;
}

MimeTypes& MimeTypes::operator=(const MimeTypes& other)
{
    (void)other;
    return *this;
}

// ============================================================================
// MIME type initialization
// ============================================================================

void MimeTypes::_initMimeTypes()
{
    // Text types
    _mimeTypes["html"] = "text/html";
    _mimeTypes["htm"] = "text/html";
    _mimeTypes["shtml"] = "text/html";
    _mimeTypes["css"] = "text/css";
    _mimeTypes["xml"] = "text/xml";
    _mimeTypes["txt"] = "text/plain";
    _mimeTypes["text"] = "text/plain";
    _mimeTypes["log"] = "text/plain";
    _mimeTypes["conf"] = "text/plain";
    _mimeTypes["ini"] = "text/plain";
    _mimeTypes["csv"] = "text/csv";
    _mimeTypes["tsv"] = "text/tab-separated-values";
    _mimeTypes["md"] = "text/markdown";
    _mimeTypes["markdown"] = "text/markdown";
    _mimeTypes["rtf"] = "text/rtf";
    _mimeTypes["vtt"] = "text/vtt";

    // JavaScript
    _mimeTypes["js"] = "application/javascript";
    _mimeTypes["mjs"] = "application/javascript";

    // JSON and related
    _mimeTypes["json"] = "application/json";
    _mimeTypes["map"] = "application/json";

    // XML variants
    _mimeTypes["rss"] = "application/rss+xml";
    _mimeTypes["atom"] = "application/atom+xml";
    _mimeTypes["xsl"] = "application/xslt+xml";
    _mimeTypes["xslt"] = "application/xslt+xml";

    // Images
    _mimeTypes["gif"] = "image/gif";
    _mimeTypes["jpeg"] = "image/jpeg";
    _mimeTypes["jpg"] = "image/jpeg";
    _mimeTypes["jpe"] = "image/jpeg";
    _mimeTypes["png"] = "image/png";
    _mimeTypes["ico"] = "image/x-icon";
    _mimeTypes["svg"] = "image/svg+xml";
    _mimeTypes["svgz"] = "image/svg+xml";
    _mimeTypes["webp"] = "image/webp";
    _mimeTypes["bmp"] = "image/bmp";
    _mimeTypes["tiff"] = "image/tiff";
    _mimeTypes["tif"] = "image/tiff";
    _mimeTypes["avif"] = "image/avif";

    // Audio
    _mimeTypes["mp3"] = "audio/mpeg";
    _mimeTypes["ogg"] = "audio/ogg";
    _mimeTypes["oga"] = "audio/ogg";
    _mimeTypes["wav"] = "audio/wav";
    _mimeTypes["weba"] = "audio/webm";
    _mimeTypes["aac"] = "audio/aac";
    _mimeTypes["flac"] = "audio/flac";
    _mimeTypes["m4a"] = "audio/mp4";
    _mimeTypes["mid"] = "audio/midi";
    _mimeTypes["midi"] = "audio/midi";
    _mimeTypes["opus"] = "audio/opus";

    // Video
    _mimeTypes["mp4"] = "video/mp4";
    _mimeTypes["m4v"] = "video/mp4";
    _mimeTypes["webm"] = "video/webm";
    _mimeTypes["ogv"] = "video/ogg";
    _mimeTypes["avi"] = "video/x-msvideo";
    _mimeTypes["mov"] = "video/quicktime";
    _mimeTypes["wmv"] = "video/x-ms-wmv";
    _mimeTypes["flv"] = "video/x-flv";
    _mimeTypes["mkv"] = "video/x-matroska";
    _mimeTypes["mpeg"] = "video/mpeg";
    _mimeTypes["mpg"] = "video/mpeg";
    _mimeTypes["3gp"] = "video/3gpp";
    _mimeTypes["3g2"] = "video/3gpp2";
    _mimeTypes["ts"] = "video/mp2t";

    // Fonts
    _mimeTypes["ttf"] = "font/ttf";
    _mimeTypes["otf"] = "font/otf";
    _mimeTypes["woff"] = "font/woff";
    _mimeTypes["woff2"] = "font/woff2";
    _mimeTypes["eot"] = "application/vnd.ms-fontobject";

    // Archives
    _mimeTypes["zip"] = "application/zip";
    _mimeTypes["gz"] = "application/gzip";
    _mimeTypes["gzip"] = "application/gzip";
    _mimeTypes["tar"] = "application/x-tar";
    _mimeTypes["tgz"] = "application/gzip";
    _mimeTypes["bz2"] = "application/x-bzip2";
    _mimeTypes["7z"] = "application/x-7z-compressed";
    _mimeTypes["rar"] = "application/vnd.rar";
    _mimeTypes["xz"] = "application/x-xz";

    // Documents
    _mimeTypes["pdf"] = "application/pdf";
    _mimeTypes["doc"] = "application/msword";
    _mimeTypes["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    _mimeTypes["xls"] = "application/vnd.ms-excel";
    _mimeTypes["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
    _mimeTypes["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    _mimeTypes["odt"] = "application/vnd.oasis.opendocument.text";
    _mimeTypes["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    _mimeTypes["odp"] = "application/vnd.oasis.opendocument.presentation";
    _mimeTypes["epub"] = "application/epub+zip";

    // Binary/executables
    _mimeTypes["bin"] = "application/octet-stream";
    _mimeTypes["exe"] = "application/octet-stream";
    _mimeTypes["dll"] = "application/octet-stream";
    _mimeTypes["so"] = "application/octet-stream";
    _mimeTypes["dmg"] = "application/octet-stream";
    _mimeTypes["iso"] = "application/octet-stream";
    _mimeTypes["img"] = "application/octet-stream";
    _mimeTypes["deb"] = "application/vnd.debian.binary-package";
    _mimeTypes["rpm"] = "application/x-rpm";
    _mimeTypes["apk"] = "application/vnd.android.package-archive";

    // Web specific
    _mimeTypes["wasm"] = "application/wasm";
    _mimeTypes["manifest"] = "application/manifest+json";
    _mimeTypes["webmanifest"] = "application/manifest+json";

    // Source code (served as text)
    _mimeTypes["c"] = "text/x-c";
    _mimeTypes["cpp"] = "text/x-c++";
    _mimeTypes["cc"] = "text/x-c++";
    _mimeTypes["cxx"] = "text/x-c++";
    _mimeTypes["h"] = "text/x-c";
    _mimeTypes["hpp"] = "text/x-c++";
    _mimeTypes["hxx"] = "text/x-c++";
    _mimeTypes["java"] = "text/x-java-source";
    _mimeTypes["py"] = "text/x-python";
    _mimeTypes["rb"] = "text/x-ruby";
    _mimeTypes["php"] = "text/x-php";
    _mimeTypes["pl"] = "text/x-perl";
    _mimeTypes["sh"] = "text/x-shellscript";
    _mimeTypes["bash"] = "text/x-shellscript";
    _mimeTypes["rs"] = "text/x-rust";
    _mimeTypes["go"] = "text/x-go";
    _mimeTypes["swift"] = "text/x-swift";
    _mimeTypes["kt"] = "text/x-kotlin";
    _mimeTypes["scala"] = "text/x-scala";
    _mimeTypes["sql"] = "text/x-sql";
    _mimeTypes["yaml"] = "text/yaml";
    _mimeTypes["yml"] = "text/yaml";
    _mimeTypes["toml"] = "text/x-toml";

    // Misc
    _mimeTypes["ics"] = "text/calendar";
    _mimeTypes["vcf"] = "text/vcard";
    _mimeTypes["swf"] = "application/x-shockwave-flash";
}

// ============================================================================
// Public methods
// ============================================================================

std::string MimeTypes::getMimeType(const std::string& extension) const
{
    // Convert extension to lowercase
    std::string ext = extension;
    for (std::string::size_type i = 0; i < ext.length(); ++i)
        ext[i] = std::tolower(static_cast<unsigned char>(ext[i]));

    // Remove leading dot if present
    if (!ext.empty() && ext[0] == '.')
        ext = ext.substr(1);

    // Look up in map
    std::map<std::string, std::string>::const_iterator it = _mimeTypes.find(ext);
    if (it != _mimeTypes.end())
        return it->second;

    return DEFAULT_MIME_TYPE;
}

std::string MimeTypes::getMimeTypeByFile(const std::string& filename) const
{
    // Find the last dot in the filename
    std::string::size_type dotPos = filename.rfind('.');
    if (dotPos == std::string::npos || dotPos == filename.length() - 1)
        return DEFAULT_MIME_TYPE;

    // Extract extension
    std::string extension = filename.substr(dotPos + 1);
    return getMimeType(extension);
}

bool MimeTypes::isTextType(const std::string& mimeType) const
{
    return (mimeType.find("text/") == 0 ||
            mimeType == "application/json" ||
            mimeType == "application/javascript" ||
            mimeType == "application/xml" ||
            mimeType.find("+xml") != std::string::npos ||
            mimeType.find("+json") != std::string::npos);
}

bool MimeTypes::isBinaryType(const std::string& mimeType) const
{
    return !isTextType(mimeType);
}

void MimeTypes::addMimeType(const std::string& extension, const std::string& mimeType)
{
    std::string ext = extension;
    for (std::string::size_type i = 0; i < ext.length(); ++i)
        ext[i] = std::tolower(static_cast<unsigned char>(ext[i]));

    // Remove leading dot if present
    if (!ext.empty() && ext[0] == '.')
        ext = ext.substr(1);

    _mimeTypes[ext] = mimeType;
}
