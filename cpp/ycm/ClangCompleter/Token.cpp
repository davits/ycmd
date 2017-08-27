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

#include "Token.h"

namespace YouCompleteMe {

namespace {

bool isStaticVariable( const CXCursor &cursor ) {
  CXCursor parent = clang_getCursorSemanticParent( cursor );
  CXCursorKind kind = clang_getCursorKind( parent );
  // Since cursor is a declaration, its parent should be a declaration too.
  return kind == CXCursor_ClassDecl || CXCursor_StructDecl;
}

bool isConstVariable( const CXCursor &cursor ) {
  CXType type = clang_getCursorType( cursor );
  CXType real_type = clang_getCanonicalType( type );
  return clang_isConstQualifiedType( real_type );
}

bool isVirtualMethod( const CXCursor &cursor ) {
  return clang_CXXMethod_isVirtual( cursor ) ||
         clang_CXXMethod_isPureVirtual( cursor );
}

} // unnamed namespace

Token::Token()
  : kind( Token::Kind::IDENTIFIER )
  , scope( Scope::NONE )
  , type( Token::Type::UNSUPPORTED )
  , range()
  , modifiers() {
}

Token::Token( const CXTokenKind cx_kind, const CXSourceRange &cx_range,
              const CXCursor &cx_cursor )
  : scope( Scope::NONE )
  , type( Type::UNSUPPORTED )
  , range( cx_range )
  , modifiers() {
  initialize( cx_kind, cx_cursor );
}

bool Token::operator==( const Token &other ) const {
  return kind == other.kind &&
         type == other.type &&
         scope == other.scope &&
         range == other.range;
}

void Token::initialize( const CXTokenKind cx_kind,
                        const CXCursor &cx_cursor ) {
  switch ( cx_kind ) {
    case CXToken_Punctuation:
      kind = Kind::PUNCTUATION;
      type = Type::PUNCTUATION;
      break;

    case CXToken_Comment:
      kind = Kind::COMMENT;
      type = Type::COMMENT;
      break;

    case CXToken_Keyword:
      kind = Kind::KEYWORD;
      type = Type::KEYWORD;
      break;

    case CXToken_Literal:
      kind = Kind::LITERAL;
      MapTypeAndScope( cx_cursor );
      break;

    case CXToken_Identifier:
      kind = Kind::IDENTIFIER;
      MapTypeAndScope( cx_cursor );
      break;
  }
}

// This is a recursive function.
// Recursive call is made for the reference cursor kind,
// with the referenced cursor as an argument,
// therefore recursion level should not exceed 2.
void Token::MapTypeAndScope( const CXCursor &cursor ) {
  CXCursorKind kind = clang_getCursorKind( cursor );

  switch ( kind ) {
    case CXCursor_IntegerLiteral:
      type = Type::INTEGER;
      break;

    case CXCursor_FloatingLiteral:
      type = Type::FLOATING;
      break;

    case CXCursor_ImaginaryLiteral:
      type = Type::IMAGINARY;
      break;

    case CXCursor_StringLiteral:
      type = Type::STRING;
      break;

    case CXCursor_CharacterLiteral:
      type = Type::CHARACTER;
      break;

    case CXCursor_Namespace:
    case CXCursor_NamespaceAlias:
    case CXCursor_NamespaceRef:
      type = Type::NAMESPACE;
      MapScope( cursor );
      break;

    case CXCursor_ClassDecl:
    case CXCursor_ClassTemplate:
      type = Type::CLASS;
      MapScope( cursor );
      break;

    case CXCursor_StructDecl:
      type = Type::STRUCT;
      MapScope( cursor );
      break;

    case CXCursor_UnionDecl:
      type = Type::UNION;
      MapScope( cursor );
      break;

    case CXCursor_TypedefDecl: // typedef
    case CXCursor_TypeAliasDecl: // using
      type = Type::TYPE_ALIAS;
      break;

    case CXCursor_FieldDecl:
      type = Type::VARIABLE;
      MapScope( cursor );
      if ( clang_CXXField_isMutable( cursor ) ) {
        modifiers.push_back( Modifier::MUTABLE );
      } else if ( isConstVariable( cursor ) ) {
        modifiers.push_back( Modifier::CONST );
      }
      break;

    case CXCursor_VarDecl:
      //scope = getVariableType( cursor );
      type = Type::VARIABLE;
      MapScope( cursor );
      // Clang reports static member variables as plain variables
      // not sure if it is a bug or feature
      if ( scope == Scope::CLASS || scope == Scope::STRUCT ) {
        modifiers.push_back( Modifier::STATIC );
      }
      if ( isConstVariable( cursor ) ) {
        modifiers.push_back( Modifier::CONST );
      }
      break;

    case CXCursor_CXXMethod:
      if ( clang_CXXMethod_isStatic( cursor ) ) {
        modifiers.push_back( Modifier::STATIC );
      } else if ( isVirtualMethod( cursor ) ) {
        modifiers.push_back( Modifier::VIRTUAL );
      }
      if ( clang_CXXMethod_isConst( cursor ) ) {
        modifiers.push_back( Modifier::CONST );
      }
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_FunctionDecl:
      type = Type::FUNCTION;
      MapScope( cursor );
      break;

    case CXCursor_ParmDecl:
      type = Type::FUNCTION_PARAMETER;
      break;

    case CXCursor_EnumDecl:
      type = Type::ENUMERATION;
      break;

    case CXCursor_EnumConstantDecl:
      type = Type::ENUMERATOR;
      break;

    case CXCursor_TemplateTypeParameter:
      type = Type::TEMPLATE_PARAMETER;
      break;

    case CXCursor_NonTypeTemplateParameter:
      type = Type::TEMPLATE_NON_TYPE_PARAMETER;
      break;

    case CXCursor_PreprocessingDirective:
      type = Type::PREPROCESSING_DIRECTIVE;
      break;

    case CXCursor_MacroDefinition:
    //case CXCursor_MacroExpansion: // Same as CXCursor_MacroInstantiation
    case CXCursor_MacroInstantiation:
      type = Type::MACRO;
      break;

    // When we have a type reference we need to do one more step
    // to find out what it is referencing.
    case CXCursor_TypeRef:
    case CXCursor_TemplateRef:
    case CXCursor_DeclRefExpr:
    case CXCursor_MemberRefExpr:
    case CXCursor_MemberRef:
    case CXCursor_VariableRef: {
      CXCursor ref = clang_getCursorReferenced( cursor );
      if ( !clang_Cursor_isNull( ref ) ) {
        MapTypeAndScope( ref );
      }
      break;
    }

    default:
      type = Type::UNSUPPORTED;
      break;
  }
}

void Token::MapScope( const CXCursor &cursor ) {
  CXCursor parent = clang_getCursorSemanticParent( cursor );
  CXCursorKind kind = clang_getCursorKind( parent );
  // Since cursor is a declaration, its parent should be a declaration too.
  switch ( kind ) {
    case CXCursor_TranslationUnit:
      scope = Scope::TRANSLATION_UNIT;
      break;
    case CXCursor_Namespace:
      scope = Scope::NAMESPACE;
      break;
    case CXCursor_ClassDecl:
      scope = Scope::CLASS;
      break;
    case CXCursor_StructDecl:
      scope = Scope::STRUCT;
      break;
    case CXCursor_UnionDecl:
      scope = Scope::UNION;
      break;
    default:
      scope = Scope::FUNCTION;
      break;
  }
}

} // YouCompleteMe
