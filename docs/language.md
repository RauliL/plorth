# Language reference

This page gives a short summary of the Plorth programming language.

## Interpreter

Plorth is interpreted programming language. It's not compiled to executables,
or even bytecode that is executed inside a virtual machine. Instead source code
in plain text will be read by the interpreter, compiled into memory as Plorth
values and then run within the interpreter process.

Once you have Plorth installed on your system, you can launch the interpreter
with `plorth` command. Path to the file containing Plorth source code that is
executed will be given as command line argument to the Plorth executable, for
example `plorth file.plorth`, if source code of the program you want to execute
is located in a file called `file.plorth` in current working directory.
Additional command line arguments can be given after the filename, which will
be available to the executed Plorth program.

You can also omit the filename (or replace it with hyphen `-`) if you want to
launch Plorth in interactive mode. This allows you to type in your Plorth
program with a built-in line editor, where each line inputted will be treated
as Plorth program and executed.

Various command line flags can also be given to the interpreter. These are:

<table>
  <tr>
    <th scope="row">-c</th>
    <td>Only checks that the syntax of the program is correct and does not
    execute the program.</td>
  </tr>
  <tr>
    <th scope="row">-f</th>
    <td>After the program has been compiled, forks the interpreter process
    into background before executing the program. This feature is not available
    on every platform. (Microsoft Windows)</td>
  </tr>
  <tr>
    <th scope="row">--version</th>
    <td>Displays version number of the Plorth interpreter and terminates the
    interpreter process.</td>
  </tr>
  <tr>
    <th scope="row">--help</th>
    <td>Displays all available command line flags.</td>
  </tr>
</table>

## Stack

Plorth is a stack based programming language. This means that the programmer
has a single <abbr title="first-in, last-out">FILO</abbr> container known as
*stack*, on which they operate on. Values are pushed into the stack and popped
out of it. Plorth does not support variables like many other programming
languages do. (Although you can use file level constant variables with `const`
word.)

Plorth is very similar to [Forth] programming language and can be considered a
Forth variant.

[Forth]: https://en.wikipedia.org/wiki/Forth_(programming_language)

## Words and dictionaries

When the interpreter encounters a whitespace separated word, such as `foo`, it
will execute the matching piece of code found in the dictionary with that same
word as the identifier.

When a word is being executed, the code to be executed is searched
through the following four steps, in this specific order:

1. Value specific words: If the stack is not empty, the word specific to the
   top-most value of the stack will be searched first. This is done through
   the prototype chain which will be described below. Most of the operations
   performed on values are usually value specific, meaning that the same word
   can execute different operations based on the top-most value of the stack.

2. Local dictionary: Each file imported into the interpreter is run inside a
   context which has a local dictionary where words declared in the file will
   be placed into. Words imported from other files (also known as modules) will
   also be placed into the local dictionary.

3. Global dictionary: The Plorth interpreter has a global dictionary of words that
   will also be searched. This dictionary contains the most basic operations in
   the Plorth programming language.

4. Number: If the word can be converted into a decimal number, it will be converted
   into a double precision floating point number and placed on the top of the stack.

If none of these steps apply, a reference error will be thrown.

## Prototypes

Each value except null have some kind of prototype object, where value specific words
will be searched from. Prototype objects might inherit from other prototype
objects, creating an inheritance chain, which will be traversed when value
specific words are being searched.

Prototype of the top-most value on the stack can be extracted with
`prototype` word found in global dictionary. On objects, it can also be found
as property identified `__proto__`. Objects which do not have `__proto__`
property will use the global `object` as prototype instead.

New object instances can be created from objects that have `prototype`
property with global word `new`. Value of this property will be placed into
`__proto__` property of the newly instantiated object. For example, if we would
have this object as the top-most value on our stack

```json
{ "prototype": { "foo": "bar" } }
```

we could create an instance of the object with `new` word. After `new` has been
called, an instance of the object will be placed as the top-most value of the
stack. This new object will have a property called `foo` available, the value of which
will be a string containing text `"bar"`.

## Data types

### Null

Null values represent an empty value or no value at all. It is usually
returned by words to indicate that nothing was found/accomplished by the word.
You can push null values into the stack with the `null` word found in global
dictionary.

### Boolean

Boolean values represent truthness. They can be placed onto the stack with
words `true` and `false` which can be found in the global dictionary.

### Number

Numbers represent whole or fractional quantities. Whole numbers also support
bitwise operations in two's complement representation.

### String

String is a container for textual data, a sequence of Unicode code points.

Strings can be manipulated with words such as `capitalize` or converted to
other types with words such as `>number`.

You can also break a string into an array of substring with `lines` or other
similar words.

### Array

Arrays are indexed sequences of other values. Arrays can be constructed with
*array literals*, which use same syntax as is used for arrays in JSON. For
example, if you would like to construct an array containing numbers from 0 to
5 and push that onto the stack, you would write:

```json
[0, 1, 2, 3, 4, 5]
```

After that, the array is available as the top-most item of the stack.

Length of an array can be retrieved with the `length` word. This places the
number of elements in the array on top of the stack, while leaving the array
itself intact.

```
[1, 2, 3] length # -> [1, 2, 3] 3
```

Items from the array can be accessed with the `@` word. You need to place the
numeric index of the item you wish to retrieve and the array itself onto the
stack and finally execute the word. If the index is out of bounds a range error
will be thrown.

```
1 [1, 2, 3] @ # -> [1, 2, 3] 2
```

### Object

Objects are associative arrays that map string keys into values.

Objects literals are constructed in the same way as in JSON.
```
{"foo": 1, "bar": "baz"}
```

Values are accessed using the `@` word.
```
"foo" {"foo": "bar"} @ # -> {"foo": "bar"} "bar"
```

Values are assigned with the `!` word.
```
"foo" "bar" {} ! # -> {"bar": "foo"}
```


### Quote

Quote is a piece of code that can be executed when required. Quotes can be
constructed in many different ways, including *quote literals* which are words
placed inside parenthesis:

```
( foo bar baz )
```

This places a quote with words `foo`, `bar` and `baz` onto the stack. When you
execute the word `call` on the quote, those three words will be executed in
sequence. Quotes are pretty much like what other languages would call
functions, except that they do not take any arguments but rather operate
directly on the stack.

Quotes can also be constructed dynamically with the `compile` keyword. It takes
source code in the form of a string, compiles it into a quote and places the
compiled quote onto the stack. Quotes can also be constructed from other quotes
by currying, composing and negation.

Quotes can usually be converted back into source code with the `>source` word,
with the exception being native core words that are built into the interpreter.

### Error

Errors are special values that will be *thrown* when some kind of errorneous
situation is encountered. Unless errors are being *caught* and processed
in some way, the execution of the program will be terminated.

Errors contain numeric error code and optional textual description of the
problem. Unlike in other languages, it is not possible to construct new types
of errors.

### Symbol

Symbols are special values that represent any kind of identifier encountered in
Plorth source code. In fact, entire Plorth programs can be represented with the
data types available in Plorth, and when Plorth program is being compiled, it's
converted from the source code form into sequence of Plorth values.

Symbols can be constructed during runtime, by converting strings into symbols.
This can be done with the `>symbol` word found in string prototype. Symbols can
be converted back to strings with the `>string` word, or executed with the
`call` word, which finds the most suitable word matching the identifier and
executes it.

### Word

Word is a pair consisting from symbol and quote. The symbol acts as the
identifier of the word, by which the word can be found from dictionary. The
quote portion of the word can be executed with `call` word found in word
prototype.

Words can also be constructed during runtime by placing symbol and quote onto
the stack and executing the `>word` word.

## Declaring new words

You can declare commonly used operations into local dictionary as words, meaning
that you can re-use piece of code by just referring to it by the name given to
it during it's declaration. This is done by using the colon (`:`), followed by
name of the word you wish the declare. All code following the colon and word
name will be considered to be part of the word, until semicolon (`;`) is
encountered which terminates the word declaration. Words are the Plorth
equivalent of what is known as *functions* in other programming languages.

```
: hello "hello there" ;
```

Above code declares word `hello` into local dictionary. The word contains single
string literal `"hello there"`, which will be placed onto the stack when the
word is being executed.

Words operate on the calling context's stack, so everything placed onto the
stack before execution of the word will be available to the word when it's being
executed. All values placed onto the stack by the word will be available to the
calling context once the word has finished it's execution.

```
: add-one 1 + ;
```

Above example will declare word which takes the top-most value from the stack
and increments it by one. If you would call this with `1 add-one`, you would end
up with value `2` being placed onto top of the stack.

Words declared to local dictionary of a file can also be imported to other files
with the `import` word.

## Conditional execution

Plorth provides builtin words in the global dictionary that allow you to execute
code conditionally.

For example
```
( "hello there" println ) if
```

Above code would execute the quote if top-most value of the stack would be
boolean value `true`. Otherwise it would be ignored.

Plorth also provides `if-else` word, which takes two quotes and executes the
first one when the top-most value of the stack is `true`, and the second one
when it's `false`.

```
1 1 + 2 = ( "1 + 1 equals 2" ) ( "something is seriously wrong" ) if-else
```

## Loops

Plorth also provides utility for executing certain piece of code repeatedly,
as long as certain criteria is being met. This is known as `while` loop. It
takes two quotes and executes the second until execution of the first one
pushes something else than `true` as top-most value of the stack. The boolean
value used as condition will be popped of the stack during each iteration.

```
10 ( dup 0 > ) ( dup println 1 - ) while
```

Above code would print numbers from 10 to 1 into the standard output.

## Catching errors

When the Plorth interpreter encounters an error situation, error value will be
*thrown*. Unless this value is catched by the program, interpreter process will
terminate itself and print the error to the standard error output.

You can recover from these situations by *catching* the errors with `try` word.

```
( "1" 1 + ) ( "You cannot add number to string in Plorth." ) try
```

When above code is executed, first quote will be run. It throws an error because
you cannot mix strings and numbers with each other. However, this error will be
caught and the second quote will be run with the error value now placed onto top
of the stack. This feature can be used to execute operations that can fail and
throw errors, allowing you to recover from the error situations if such is
encountered.

You can also throw errors yourself, with the `throw` word to indicate that
something went wrong.

```
"Oops." value-error throw
```

Above code constructs an value error with message `"Oops."` and places this
error value as the top-most value of the stack. This error will be then popped
out of the stack and thrown.

Available error types are:

<table>
  <tr>
    <th scope="row"><kbd>type-error</kbd></th>
    <td>When given value is of incorrect type.</td>
  </tr>
  <tr>
    <th scope="row"><kbd>value-error</kbd></th>
    <td>When given value is otherwise incorrect.</td>
  </tr>
  <tr>
    <th scope="row"><kbd>range-error</kbd></th>
    <td>When given value is out of supported range. For example array and string
    indexes.</td>
  </tr>
  <tr>
    <th scope="row"><kbd>unknown-error</kbd></th>
    <td>When the error situation does not meet criteria of anything mentioned
    above.</td>
  </tr>
</table>

## Modules

Plorth program can import words from other files known as *modules*. When the
word `import` is executed, the interpreter attempts to find a file matching the
path from the top-most value of the stack.

For example:

```
# file1.plorth

: hello-world "Hello, World" println ;
```

```
# file2.plorth

"./file1.plorth" import

# Word "hello-world" is now placed into local dictionary of this file, and can
# be executed.
hello-world
```

File extension can be omitted, in which case the interpreter looks for a file
with matching file name and `.plorth` extension. If the imported path points to
a directory in the file system that contains file `index.plorth`, this file will
be imported, allowing you to organize your codebase into directories known as
*packages*.

If non-absolute path is given, Plorth interpreter will search matching file from
current working and directory as well as directories defined in environment
variable `PLORTHPATH`. By default, `PLORTHPATH` will point to the directory
where Plorth runtime library was installed during installation of Plorth.
