/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef binio_h
#define binio_h

#include <cstdarg>
#include <cstdio>
#include <cstdint>

#if defined(__cplusplus)
extern "C" {
#endif

    /*
    Notes on format of format strings:
    * whitespace is ignored
    * each "group" consists of an optional count (defaults to 1),
    an optional byte-order marker (defaults to H, "host-native"),
    and a  data-type specifier.
    * when unpacking, each variable argument is a pointer to the
    appropriate number of Object::objects of the appropriate type.
    * when packing, each variable argument is an object of the
    appropriate type if the count is omitted, or a pointer to the
    appropriate number of Object::objects of the appropriate type if the
    count is specified.
    * the buffer supplied to pack/unpack must be of sufficient length
    to hold all the data, or the behavior is unspecified.
    * the file provided to the "f" versions of the functions must be
    open in the appropriate mode, or the behavior is unspecified.
    * the file supplied to funpackf must be of sufficient length to
    hold all the data, or the behavior is unspecified.
    * the behavior of all functions is unspecified if the format string
    is incorrectly-formed.

    Data-type specifiers:
    x skipped byte; no corresponding argument
    b byte
    s two-byte two's-complement integer
    i four-byte two's-complement integer
    l eight-byte two's-complement integer
    f four-byte IEEE754 float
    d eight-byte IEEE754 double

    Byte-order specifiers:
    L little-endian
    B big-endian
    H host's native byte order
    N network byte order
    */

// Fixed-width integer types are standard in C++. float32_t/float64_t are
// optional in <cstdint>, so spell the float widths out explicitly.
#include <cstdint>

#include <exception>
#include <string>

/**
 * Thrown by the file-based readers when the stream ends before the requested
 * record has been read. Previously these reads were unchecked, so truncated or
 * corrupt files silently filled caller variables with uninitialised heap.
 */
struct TruncatedFileException : public std::exception
{
    std::string errorText;

    explicit TruncatedFileException(const std::string& what)
        : errorText(what)
    {
    }

    const char* what() const noexcept override
    {
        return errorText.c_str();
    }
};

    typedef struct {
        double   d;
        uint64_t l;
        int      i;
        float    f;
        uint16_t s;
        uint8_t  b;
    }
    test_data;

    extern void packf    (                    const char *format, ...);
    extern void spackf   (void *buffer,       const char *format, ...);
    extern void fpackf   (FILE *file,         const char *format, ...);
    extern void vspackf  (void *buffer,       const char *format, va_list args);
    extern void vfpackf  (FILE *file,         const char *format, va_list args);

    /*
     * These throw TruncatedFileException (or bad_alloc), so they are declared
     * noexcept(false) explicitly: without it MSVC infers a non-throwing
     * specification and warns C4297 at the throw site.
     */
    extern void unpackf  (                    const char *format, ...) noexcept(false);
    extern void sunpackf (const void *buffer, const char *format, ...) noexcept(false);
    extern void funpackf (FILE       *file,   const char *format, ...) noexcept(false);
    extern void vsunpackf(const void *buffer, const char *format, va_list args) noexcept(false);
    extern void vfunpackf(FILE       *file,   const char *format, va_list args) noexcept(false);

    /*
     * Like funpackf(), but returns false instead of throwing when the stream
     * ends before the record has been read. Use for optional trailing data in
     * older files; the caller supplies its own defaults.
     */
    extern bool tryfunpackf (FILE       *file,   const char *format, ...) noexcept(false);
    extern bool vtryfunpackf(FILE       *file,   const char *format, va_list args) noexcept(false);

#ifdef _MSC_VER
#ifndef va_copy
#define va_copy(dest,src) do { dest = src; } while (0)
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif

