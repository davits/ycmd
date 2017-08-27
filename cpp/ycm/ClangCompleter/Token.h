// Copyright (C) 2016 Davit Samvelyan
//
// This file is part of ycmd.
//
// ycmd is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ycmd is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ycmd.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TOKEN_H_IC9ZDM5T
#define TOKEN_H_IC9ZDM5T

#include "Range.h"

#include <string>
#include <vector>

#include <clang-c/Index.h>

namespace YouCompleteMe {

/// Represents single semantic token as a (Kind, Type) enum pair corresponding
/// to the clang's CXTokenKind and CXCursorKind enums.
struct Token {

  enum class Kind {
    PUNCTUATION = 0,
    COMMENT,
    KEYWORD,
    LITERAL,
    IDENTIFIER,
  };

  enum class Scope {
    NONE,
    TRANSLATION_UNIT,
    NAMESPACE,
    CLASS,
    STRUCT,
    UNION,
    FUNCTION,
  };

  enum class Type {
    // Punctuation types
    PUNCTUATION = 0,

    // Comment types
    COMMENT,

    // Keyword types
    KEYWORD,

    // Literal types
    // true/false are keywords
    INTEGER,
    FLOATING,
    IMAGINARY,
    STRING,
    CHARACTER,

    // Identifier types
    NAMESPACE,
    CLASS,
    STRUCT,
    UNION,
    TYPE_ALIAS,
    VARIABLE,
    FUNCTION,
    FUNCTION_PARAMETER,
    ENUMERATION,
    ENUMERATOR,
    TEMPLATE_PARAMETER,
    TEMPLATE_NON_TYPE_PARAMETER,
    PREPROCESSING_DIRECTIVE,
    MACRO,

    // Identifier without mapping to the Type enum.
    UNSUPPORTED
  };

  enum class Modifier {
    STATIC,
    VIRTUAL,
    CONST,
    MUTABLE,
  };

  Token();

  Token( const CXTokenKind kind, const CXSourceRange &tokenRange,
         const CXCursor &cursor );

  bool operator==( const Token &other ) const;

  Kind kind;

  Scope scope;

  Type type;

  Range range;

  std::vector< Modifier > modifiers;

private:

  void initialize( const CXTokenKind kind, const CXCursor &cursor );

  void MapTypeAndScope( const CXCursor &cursor );

  void MapScope( const CXCursor &cursor );

};

} // YouCompleteMe

#endif /* end of include guard: TOKEN_H_IC9ZDM5T */
