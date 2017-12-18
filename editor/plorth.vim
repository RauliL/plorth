" Vim syntax file
" Language: Plorth
" Maintainer: Rauli Laine
" Last Change: 2017 Dec 18

if !exists("main_syntax")
  " Quit if a syntax file has already been loaded.
  if exists("b:current_syntax")
    finish
  endif
  let main_syntax = "plorth"
elseif exists("b:current_syntax") && b:current_syntax == "plorth"
  finish
endif

syn iskeyword !,$,%,&,*,+,-,.,47-57,60-90,\,94-122,|,~

" Syntax: Comments
syn keyword plorthTodo contained TODO FIXME XXX NOTE
syn match plorthSharpBang "\%^#!.*" display
syn match plorthComment "#.*$" contains=plorthTodo

" Syntax: Keywords
syn keyword plorthBoolean true false
syn keyword plorthConstant null e pi inf -inf
syn keyword plorthConditional if if-else
syn keyword plorthRepeat while times
syn keyword plorthException throw try try-else

" Syntax: Definitions
syn match plorthDefinition /:\s*\k\+/
syn match plorthEndOfDefinition /;/
syn keyword plorthConstantDefinition const

" Syntax: Strings
syn match plorthStringEscape "\\["'\\/btnfr]" contained
syn match plorthStringEscape "\\u\x\{4}" contained
syn region plorthString start=/"/ end=/"/ contains=plorthStringEscape
syn region plorthString start=/'/ end=/'/ contains=plorthStringEscape

" Syntax: Numbers
syn match plorthNumber /\<[+-]\?\d\+\(.\d\+\)\?\([eE]\d\+\([+-]\d\+\)\?\)\?\>/

" Syntax: Delimiters
syn match plorthDelimiter /[()\[\]{}]/

" Syntax: Operators
syn keyword plorthOperator nop clear depth drop 2drop dup 2dup nip over rot
syn keyword plorthOperator swap tuck new typeof instance-of?

hi def link plorthTodo               Todo
hi def link plorthSharpBang          PreProc
hi def link plorthComment            Comment
hi def link plorthStringEscape       Special
hi def link plorthString             String
hi def link plorthStringDouble       Double
hi def link plorthNumber             Number
hi def link plorthBoolean            Boolean
hi def link plorthConstant           Constant
hi def link plorthConditional        Conditional
hi def link plorthRepeat             Repeat
hi def link plorthException          Exception
hi def link plorthOperator           Operator
hi def link plorthDelimiter          Delimiter
hi def link plorthConstantDefinition Define
hi def link plorthDefinition         Define
hi def link plorthEndOfDefinition    Define

let b:current_syntax = "plorth"
if main_syntax == "plorth"
  unlet main_syntax
endif
