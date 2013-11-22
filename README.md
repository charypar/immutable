# Immutable

Immutable persistent data structures for C++.

## About

Immutable is an experiment in learning about immutable persistent data structures used for example in Clojure,
as well as details of the C++ programming language. As a start it implements an immutable persistent map.

## Rationale

Immutable persistent data structures are collections that never change. Each time you add or remove an item,
you get a new collection. The old version is still available and retains its performance characteristics.
This becomes important in parallel processing, because concurrent writes cannot conflict.

The collection essentially becomes a value, the same way a number is. The trick preventing the necessity
of copying is structural sharing, which is why all of the data structures are implemented as trees.

## Usage

Immutable is trying to follow the standard library interface to collections closely, except for cases where that
isn't possible. Consider a persistent map example:

```cpp
#include "immutable"

using str_str_map = immutable::map<std::string, std::string>;

str_str_map phone_book {{"Bob", "555-5745"}, {"John", "555-2547"}};

phone_book.at("Bob"); // => "555-5745", const reference

auto updated_phone_book = phone_book.set("Dave", "555-12345");

&phone_book == &updated_phone_book; // => false

phone_book.at("Dave"); // => throws out_of_range
updated_phone_book.at("Dave"); // => "555-123456"
```

## TODO for immutable map

*  Basic ~~insert, lookup~~ and delete for unordered_map
*  Iteration support for unordered_map
*  Full std::unordered_map interface support

## TODO for other data structures

*  Immutable vector

## License

Immutable is released under the [MIT License](http://www.opensource.org/licenses/MIT).
