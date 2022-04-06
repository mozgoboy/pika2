//
// This file is part of the pika parser reference implementation:
//
//     https://github.com/lukehutch/pikaparser
//
// The pika parsing algorithm is described in the following paper: 
//
//     Pika parsing: reformulating packrat parsing as a dynamic programming algorithm solves the left recursion
//     and error recovery problems. Luke A. D. Hutchison, May 2020.
//     https://arxiv.org/abs/2005.06444
//
// This software is provided under the MIT license:
//
// Copyright 2020 Luke A. D. Hutchison
//  
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#pragma once
#include <stdlib.h>
#include "str_switch.hpp"

/** String utilities. */
class StringUtils {
private: 
    static const char NON_ASCII_CHAR = '■';

    /** Replace non-ASCII/non-printable char with a block. */
public:
    static char replaceNonASCII(char c) {
        return c < 32 || c > 126 ? NON_ASCII_CHAR : c;
    }

    /** Replace all non-ASCII/non-printable characters with a block. */
    static void replaceNonASCII(string str, string buf) {
        for (int i = 0; i < str.length(); i++) {
            char c = str[i];
            buf.append(to_string(replaceNonASCII(c)));
        }
    }

    /** Replace all non-ASCII/non-printable characters with a block. */
    static string replaceNonASCII(string str) {
        string buf;
        replaceNonASCII(str, buf);
        return buf;
    }

    /** Convert a hex digit to an integer. */
    static int hexDigitToInt(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        cout << "Illegal hex digit: " + c;
        abort();
    }

    /** Unescape a single character. */
    static char unescapeChar(string escapedChar) {
        if (escapedChar.length() == 0) {
            cout << "Empty char string";
            abort();
        }
        else if (escapedChar.length() == 1) {
            return escapedChar[0];
        }
        SWITCH (escapedChar) {
        CASE ("\\t"):
            return '\t';
        CASE ("\\b"):
            return '\b';
        CASE ("\\n"):
            return '\n';
        CASE ("\\r" ):
            return '\r';
        CASE  ("\\f" ):
            return '\f';
        CASE  ("\\'") :
            return '\'';
        CASE ("\\\""):
            return '"';
        CASE ("\\\\") :
            return '\\';
        DEFAULT :
            if ((escapedChar.substr(0,2) == "\\u") && escapedChar.length() == 6) {
                int c0 = hexDigitToInt(escapedChar[2]);
                int c1 = hexDigitToInt(escapedChar[3]);
                int c2 = hexDigitToInt(escapedChar[4]);
                int c3 = hexDigitToInt(escapedChar[5]);
                return (char)((c0 << 12) | (c1 << 8) | (c2 << 4) | c3);
            }
            else {
                cout << "Invalid character: " + escapedChar;
                abort();
            }
        }
    }

    /** Get the sequence of (possibly escaped) characters in a char range string. */
    static vector<string> getCharRangeChars(string str) {
        vector<string> charRangeChars;
        for (int i = 0; i < str.length(); i++) {
            char c = str[i];
            if (c == '\\') {
                if (i == str.length() - 1) {
                    // Should not happen
                    cout << "Got backslash at end of quoted string";
                    abort();
                }
                if (str[i + 1] == 'u') {
                    if (i > str.length() - 6) {
                        // Should not happen
                        cout << "Truncated Unicode character sequence";
                        abort();
                    }
                    charRangeChars.push_back(to_string(unescapeChar(str.substr(i, 6))));
                    i += 5; // Consume escaped characters
                }
                else {
                    string escapeSeq = str.substr(i, 2);
                    if (escapeSeq == "\\-" || escapeSeq == "\\^" || escapeSeq == "\\]"
                        || escapeSeq == "\\\\") {
                        // Preserve range-specific escaping for char ranges
                        charRangeChars.push_back(escapeSeq);
                    }
                    else {
                        charRangeChars.push_back(to_string(unescapeChar(escapeSeq)));
                    }
                    i++; // Consume escaped character
                }
            }
            else {
                charRangeChars.push_back(to_string(c));
            }
        }
        return charRangeChars;
    }

    /** Unescape a string. */
    static string unescapeString(string str) {
        string buf;
        for (int i = 0; i < str.length(); i++) {
            char c = str[i];
            if (c == '\\') {
                if (i == str.length() - 1) {
                    // Should not happen
                    cout << "Got backslash at end of quoted string";
                    abort();
                }
                if (str[i + 1] == 'u') {
                    if (i > str.length() - 6) {
                        // Should not happen
                        cout << "Truncated Unicode character sequence";
                    }
                    buf.append(to_string(unescapeChar(str.substr(i, 6))));
                    i += 5; // Consume escaped characters
                }
                else {
                    string escapeSeq = str.substr(i,  2);
                    buf.append(to_string(unescapeChar(escapeSeq)));
                    i++; // Consume escaped character
                }
            }
            else {
                buf.append(to_string(c));
            }
        }
        return buf;
    }

    /** Escape a character. */
    static string escapeChar(char c) {
        if (c >= 32 && c <= 126) {
            return to_string(c);
        }
        else if (c == '\n') {
            return "\\n";
        }
        else if (c == '\r') {
            return "\\r";
        }
        else if (c == '\t') {
            return "\\t";
        }
        else if (c == '\f') {
            return "\\f";
        }
        else if (c == '\b') {
            return "\\b";
        }
        else {
            char* buf;
            sprintf(buf, "%04x", (int)c);
            string buff(buf);
            return "\\u" + buff;
        }
    }

    /** Escape a single-quoted character. */
    static string escapeQuotedChar(char c) {
        if (c == '\'') {
            return "\\'";
        }
        else if (c == '\\') {
            return "\\\\";
        }
        else {
            return escapeChar(c);
        }
    }

    /** Escape a character. */
    static string escapeQuotedStringChar(char c) {
        if (c == '"') {
            return "\\\"";
        }
        else if (c == '\\') {
            return "\\\\";
        }
        else {
            return escapeChar(c);
        }
    }

    /** Escape a character for inclusion in a character range pattern. */
    static string escapeCharRangeChar(char c) {
        if (c == ']') {
            return "\\]";
        }
        else if (c == '^') {
            return "\\^";
        }
        else if (c == '-') {
            return "\\-";
        }
        else if (c == '\\') {
            return "\\\\";
        }
        else {
            return escapeChar(c);
        }
    }

    /** Escape a string. */
    static string escapeString(string str) {
        string buf;
        for (int i = 0; i < str.length(); i++) {
            char c = str[i];
            buf.append(c == '"' ? "\\\"" : escapeQuotedStringChar(c));
        }
        return buf;
    }
};
