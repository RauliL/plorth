# Language reference

This page gives short summary of the Plorth programming language.

## Interpreter

TODO

## Stack

TODO

## Words and dictionaries

When the interpreter encounters whitespace separated word, such as `foo`, it
will execute matching piece of code found from the dictionary with that same
word as identifier.

When word is being executed, the code that is going to be executed is searched
through these four steps, in this specific order:

1. Value specific words: If the stack is not empty, word specific to the
   top-most value of the stack will be searched first. This is done through
   prototype chain which will be described below. Most of the operations
   performed on values are usually value specific, meaning that the same word
   can execute different operations based on the top-most value of the stack.

2. Local dictionary:

3. Global dictionary: Plorth interpreter has global dictionary of words that
   will also be searched. This dictionary contains the most basic operations in
   the Plorth programming language.

4. Number: If the word can be converted into floating point decimal number, it
   will be converted into one and placed on the top of the stack.

If none of these steps provide any kind of result, reference error will be
thrown.

## Prototypes

Each value except null have some kind of prototype object, where value specific
will be searched from. Prototype objects might inherit from other prototype
objects, creating an inheritance chain, which will be traversed when value
specific words are being searched.

Prototype of the top-most value on the stack can be extracted with
`prototype` word found in global dictionary. On objects, it can also be found
as property identified as `__proto__`. Objects which do not have `__proto__`
property will use the global `object` as prototype instead.

New object instances can be created from objects that have `prototype`
property with global word `new`. Value of this property will be placed into
`__proto__` property of the newly instantiated object. For example, if we would
have this object as the top-most value on our stack

```json
{ "prototype": { "foo": "bar" } }
```

we could create an instance of the object with `new` word. After `new` has been
called, instance of the object will be placed as the top-most value of the
stack. This new object will have property called `foo` available, which value
will be a string containing text `"bar"`.

## Data types

### Null

Null values represent an empty value or no value at all. They are usually
returned by words to indicate that nothing was found/accomplished by the word.
You can push null values into the stack with the `null` word found in global
dictionary.

### Boolean

Boolean values represent truthness. They can be placed into the stack with
words `true` and `false` which can be found in the global dictionary.

### Number

TODO

### String

String is container for textual data, a sequence of Unicode code points.

TODO

### Array

Arrays are indexed sequences of other values. Arrays can be constructed with
*array literals*, which use same syntax as is used for arrays in JSON. For
example, if you would like to construct and array containing numbers from 0 to
5 and push that into the stack, you would write:

```json
[0, 1, 2, 3, 4, 5]
```

After that, the array is available as the top-most item of the stack.

Length of an array can be retrieved with `length` word. This places the number
of elements in the array as top of the stack, while leaving the array itself
intact.

```
[1, 2, 3] length # -> [1, 2, 3] 3
```

Items from the array can be accessed with `@` word. You need to place numeric
index of the item you wish to retrieve, the array itself and finally execute
the word. If the index is not out of bounds, range error will be thrown.

```
1 [1, 2, 3] @ # -> [1, 2, 3] 2
```

### Object

Objects are associative arrays that map string keys into values.

TODO: Information about object literals and how to access properties.

### Quote

Quote is piece of code that can be executed when required. Quotes can be
constructed in many different ways, including *quote literals* which are words
placed inside parenthesis:

```
( foo bar baz )
```

This places a quote with words `foo`, `bar` and `baz` into the stack. When you
execute the word `call` on the quote, those three words will be executed in
sequence. Quotes are pretty much like what other languages would call
functions, except that they do not take any arguments but rather operate
directly on the stack.

Quotes can also be constructed dynamically with `compile` keyword. This takes
source code in form of string, compiles it into a quote and places the compiled
quote into the stack. Quotes can also be constructed from other quotes by
currying, composing and negation.

Quotes can usually be converted back into source code with `>source` word,
with the exception being native core words that built in the interpreter.

### Error

Errors are special values that will be *thrown* when some kind of errorneous
situation is being encountered. Unless errors are being *caught* and processed
in some way, the execution of the program will be terminated.

Errors contain numeric error code and optional textual description of the
problem. Unlike in other languages, it is not possible to construct new types
of errors.

## Declaring new words

TODO

## Conditional execution

TODO

## Catching errors

TODO

## Modules

TODO
