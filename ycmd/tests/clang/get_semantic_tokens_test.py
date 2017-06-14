# Copyright (C) 2016 Davit Samvelyan
#
# This file is part of ycmd.
#
# ycmd is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ycmd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ycmd.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import absolute_import
from __future__ import unicode_literals
from __future__ import print_function
from __future__ import division
from future import standard_library
standard_library.install_aliases()
from builtins import *  # noqa

from nose.tools import eq_
from hamcrest import assert_that, contains, has_items

from ycmd.tests.clang import PathToTestFile, SharedYcmd
from ycmd.tests.test_utils import BuildRequest
from ycmd.responses import BuildSemanticTokenData, Range, Location
from ycmd.semantic_token import SemanticToken
from ycmd.utils import ReadFile
import requests


_TEST_FILE = PathToTestFile( 'token_test_data', 'GetTokens_Clang_test.cc' )


@SharedYcmd
def setUpModule( app ):
  app.post_json( '/load_extra_conf_file', {
    'filepath': PathToTestFile( 'token_test_data', '.ycm_extra_conf.py' ) } )
  request = {
    'filetype': 'cpp',
    'filepath': _TEST_FILE,
    'event_name': 'FileReadyToParse',
    'contents': ReadFile( _TEST_FILE )
  }
  app.post_json( '/event_notification', BuildRequest( **request ),
                 expect_errors = False )


def _BuildTokenData( kind, type, sl, sc, el, ec ):
  token = SemanticToken( kind, type,
                         Range( Location( sl, sc, _TEST_FILE ),
                                Location( el, ec, _TEST_FILE ) )  )
  return BuildSemanticTokenData( token )


def _RunTest( app, start_line, start_column, end_line, end_column, expect ):
  request = {
    'filetypes': 'cpp',
    'filepath': _TEST_FILE,
    'range': {
      'start': {
        'line_num': start_line,
        'column_num': start_column,
      },
      'end': {
        'line_num': end_line,
        'column_num': end_column,
      },
    }
  }
  response = app.post_json( '/semantic_tokens', BuildRequest( **request ),
                            expect_errors = False )

  eq_( response.status_code, requests.codes.ok )
  assert_that( response.json[ 'tokens' ], expect )


@SharedYcmd
def InvalidFile_test( app ):
  request = {
    'filetypes': 'cpp',
    'filepath': '',
    'range': {
      'start': {
        'line_num': 1,
        'column_num': 1,
      },
      'end': {
        'line_num': 1,
        'column_num': 1,
      },
    }
  }
  response = app.post_json( '/semantic_tokens', BuildRequest( **request ),
                            expect_errors = True )
  eq_( response.status_code, requests.codes.server_error )


@SharedYcmd
def PreprocessingTokens_test( app ):
  _RunTest( app, 1, 1, 4, 22,
            has_items(
              _BuildTokenData( 'Punctuation', 'Punctuation', 1, 1, 1, 2 ),
              _BuildTokenData( 'Identifier', 'PreprocessingDirective',
                               1, 2, 1, 8 ),
              _BuildTokenData( 'Identifier', 'Macro', 1, 9, 1, 11 ),
              # Literals in preprocessing directives are reported as
              # Macro definitions by clang (FIX when fixed in clang).
              _BuildTokenData( 'Literal', 'Macro', 1, 12, 1, 17 ),
              _BuildTokenData( 'Identifier', 'Macro', 2, 9, 2, 15 ),
              _BuildTokenData( 'Identifier', 'Macro', 3, 9, 3, 13 ),
              _BuildTokenData( 'Identifier', 'Macro', 3, 20, 3, 22 ),
              _BuildTokenData( 'Identifier', 'Macro', 3, 25, 3, 31 ),
              _BuildTokenData( 'Identifier', 'Macro', 4, 20, 4, 24 ),
            ) )


@SharedYcmd
def DeclarationTokens_test( app ):
  _RunTest( app, 6, 1, 42, 17,
            has_items(
              _BuildTokenData( 'Identifier', 'Function', 6, 6, 6, 20 ),
              _BuildTokenData( 'Identifier', 'Namespace', 7, 11, 7, 13 ),

              _BuildTokenData( 'Comment', 'Comment', 8, 1, 11, 4 ),

              _BuildTokenData( 'Identifier', 'TemplateParameter',
                               12, 17, 12, 18 ),
              _BuildTokenData( 'Identifier', 'TemplateNonTypeParameter',
                               12, 24, 12, 28 ),
              _BuildTokenData( 'Identifier', 'Class', 13, 7, 13, 10 ),
              _BuildTokenData( 'Identifier', 'MemberFunction', 16, 3, 16, 6 ),
              _BuildTokenData( 'Identifier', 'TemplateParameter',
                               16, 7, 16, 8 ),
              _BuildTokenData( 'Identifier', 'MemberVariable',
                               16, 12, 16, 13 ),
              _BuildTokenData( 'Identifier', 'TemplateNonTypeParameter',
                               16, 16, 16, 20 ),
              _BuildTokenData( 'Identifier', 'MemberFunction', 17, 4, 17, 7 ),
              _BuildTokenData( 'Identifier', 'MemberFunction', 19, 8, 19, 17 ),
              _BuildTokenData( 'Identifier', 'TemplateParameter',
                               19, 18, 19, 19 ),
              _BuildTokenData( 'Identifier', 'FunctionParameter',
                               19, 20, 19, 23 ),
              _BuildTokenData( 'Identifier', 'MemberVariable', 20, 5, 20, 6 ),
              _BuildTokenData( 'Identifier', 'FunctionParameter',
                               20, 9, 20, 12 ),
              _BuildTokenData( 'Identifier', 'TemplateParameter',
                               24, 3, 24, 4 ),
              _BuildTokenData( 'Identifier', 'MemberVariable', 24, 5, 24, 6 ),

              _BuildTokenData( 'Identifier', 'Class', 27, 9, 27, 12 ),
              _BuildTokenData( 'Identifier', 'TypeAlias', 27, 21, 27, 27 ),

              _BuildTokenData( 'Identifier', 'Structure', 29, 8, 29, 10 ),

              _BuildTokenData( 'Identifier', 'Enumeration', 31, 6, 31, 8 ),

              _BuildTokenData( 'Identifier', 'Enumerator', 32, 3, 32, 13 ),
              _BuildTokenData( 'Identifier', 'Enumerator', 33, 3, 33, 13 ),

              _BuildTokenData( 'Identifier', 'Union', 36, 7, 36, 9 ),
              _BuildTokenData( 'Comment', 'Comment', 36, 10, 36, 25 ),
            ) )


@SharedYcmd
def LiteralTokens_test( app ):
  _RunTest( app, 48, 1, 51, 24,
            has_items(
              _BuildTokenData( 'Literal', 'Integer', 48, 11, 48, 14 ),
              _BuildTokenData( 'Literal', 'Floating', 49, 13, 49, 18 ),
              _BuildTokenData( 'Literal', 'Character', 50, 12, 50, 15 ),
              _BuildTokenData( 'Literal', 'String', 51, 19, 51, 24 ),
            ) )


@SharedYcmd
def DetailedUsageTokens_test( app ):
  _RunTest( app, 53, 1, 54, 28,
            contains(
              _BuildTokenData( 'Identifier', 'Namespace', 53, 3, 53, 5 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 5, 53, 7 ),
              _BuildTokenData( 'Identifier', 'TypeAlias', 53, 7, 53, 13 ),
              _BuildTokenData( 'Identifier', 'Variable', 53, 14, 53, 17 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 18, 53, 19 ),
              _BuildTokenData( 'Identifier', 'Namespace', 53, 20, 53, 22 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 22, 53, 24 ),
              _BuildTokenData( 'Identifier', 'TypeAlias', 53, 24, 53, 30 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 30, 53, 31 ),
              _BuildTokenData( 'Identifier', 'FunctionParameter',
                               53, 31, 53, 35 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 35, 53, 36 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 53, 36, 53, 37 ),

              _BuildTokenData( 'Identifier', 'Namespace', 54, 3, 54, 5 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 54, 5, 54, 7 ),
              _BuildTokenData( 'Identifier', 'Enumeration', 54, 7, 54, 9 ),
              _BuildTokenData( 'Identifier', 'Variable', 54, 10, 54, 11 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 54, 12, 54, 13 ),
              _BuildTokenData( 'Identifier', 'Namespace', 54, 14, 54, 16 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 54, 16, 54, 18 ),
              _BuildTokenData( 'Identifier', 'Enumerator', 54, 18, 54, 28 ),
              _BuildTokenData( 'Punctuation', 'Punctuation', 54, 28, 54, 29 ),
            ) )


@SharedYcmd
def UnicodeTokens_test( app ):
  _RunTest( app, 56, 1, 59, 10,
            has_items(
              _BuildTokenData( 'Comment', 'Comment', 56, 3, 56, 22 ),
              _BuildTokenData( 'Identifier', 'TypeAlias', 57, 18, 57, 24 ),
              _BuildTokenData( 'Identifier', 'TypeAlias', 58, 3, 58, 9 ),
              _BuildTokenData( 'Identifier', 'Variable', 58, 10, 58, 12 ),
              _BuildTokenData( 'Identifier', 'MemberVariable', 59, 6, 59, 7 ),
            ) )


@SharedYcmd
def StaticMemberTokens_test( app ):
  _RunTest( app, 63, 1, 75, 15,
            has_items(
              _BuildTokenData( 'Identifier', 'GlobalVariable', 63, 12, 63, 29 ),
              _BuildTokenData( 'Identifier', 'Function', 64, 12, 64, 30 ),

              _BuildTokenData( 'Identifier', 'MemberVariable', 68, 13, 68, 14 ),
              _BuildTokenData( 'Identifier', 'StaticMemberVariable',
                               69, 20, 69, 21 ),

              _BuildTokenData( 'Identifier', 'MemberFunction', 71, 14, 71, 17 ),
              _BuildTokenData( 'Identifier', 'StaticMemberFunction',
                               72, 21, 72, 24 ),

              _BuildTokenData( 'Identifier', 'StaticMemberVariable',
                               75, 10, 75, 11 ),
            ) )
