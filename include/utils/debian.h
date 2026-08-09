/**
 * @file debian.h
 * @brief Extraction of Debian (.deb) packages.
 *
 * This header declares the deb_extract() function, which unpacks a
 * Debian package into a target directory, handling both the control
 * and data archives.
 */

#ifndef DEBIAN_H
#define DEBIAN_H

/**
 * @brief Extracts a Debian package into the target directory.
 *
 * Unpacks the .deb archive at the given path. The control archive is
 * extracted to a "DEBIAN" subdirectory while the data archive is
 * extracted with original file permissions preserved.
 *
 * @param package The path to the .deb package file.
 * @param target The directory where the package contents are extracted.
 * @return true if the package was successfully extracted, false otherwise.
 */
bool deb_extract(const char* package, const char* target);

#endif // DEBIAN_H
