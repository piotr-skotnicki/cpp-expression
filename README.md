cpp-expression
==============

Introduction
------------

`cpp-expression` is a single-header, header-only library, offering Boost.Lambda-like syntax for declaring lambda expressions:

```cpp
std::for_each(begin(v), end(v), std::cout << _1);
```

Basics
------

`cpp-expression` builds up on the idea of expression templates. The library introduces three fundamental types:

- `expr::constant`
- `expr::variable`
- `expr::placeholder`

Any unary or binary operation, involving expressions of the aforementioned types, automatically becomes a delayed function, that is:

```cpp
std::cout << _1;
```

is not executed until a function call operator is applied:

```cpp
auto lambda = std::cout << _1;
lambda(3.14f); // prints 3.14f
```

`expr::_1`, `expr::_2`, `...`, `expr::_7` (or just `_1`, `_2`) are so-called *placeholders* that represent an actual argument from a delayed function call, and themselves are delayed functions, that is:

```cpp
_1(1, 2, 3, 4); // returns 1
_2(1, 2, 3, 4); // returns 2
_3(1, 2, 3, 4); // returns 3
_4(1, 2, 3, 4); // returns 4
```

`expr::variable` holds a reference to the wrapped object and itself is a delayed function, delegating any operation to that object:

```cpp
int i = 0;
auto inc = ++expr::variable(i);
inc(); // i equals 1
inc(); // i equals 2
```

`expr::constant` holds a copy of the passed object and itself is a delayed function, useful to overcome the problem of operator precedence:

```cpp
auto lambda = std::cout << expr::constant('\n') << _1;
```

Had `expr::constant` not been used, the compiler would first execute `std::cout << '\n'`, printing a new line character to the standard output, and then construct a delayed function of the form `std::cout << _1` (that is, without a leading new line character).

> Hint: When in doubt, call the `expr::expressify(e)` helper function:

`cpp-expression` allows short-circuit evaluation of operands, making the below code safe:

```cpp
(_1 != nullptr && *_1 == 42)(nullptr);
```

Usage
-----

#### Including the header file:

```cpp
#include "expression.hpp"
```

#### Using fully qualified names:

```cpp
expr::variable(a)++, std::cout << expr::constant('\n') << expr::_1 << expr::_2;
```

#### Importing placeholders only:

```cpp
using namespace expr::placeholders;
expr::variable(a)++, std::cout << expr::constant('\n') << _1 << _2;
```

#### Importing all symbols:

```cpp
using namespace expr;
variable(a)++, std::cout << constant('\n') << _1 << _2;
```

Examples
--------

```cpp
std::for_each(begin(v), end(v), std::cout << _1 << '\n');
```

```cpp
std::sort(begin(v), end(v), _1 > _2);
```

```cpp
std::sort(begin(v), end(v), (_1->*&A::foo)() > (_2->*&A::foo)());
```

```cpp
std::any_of(begin(ptrs), end(ptrs), _1 != nullptr && *_1 == 5);
```

```cpp
std::transform(begin(a), end(a), begin(b), begin(c), _1 + _2);
```

```cpp
int sum = 0;
std::for_each(begin(v), end(v), sum += _1);
```

```cpp
std::for_each(begin(a), end(a), std::cout << constant('\n') << _1);
```

```cpp
std::for_each(begin(indices), end(indices), ++variable(array)[_1]);
```

```cpp
void foo(int i, char c);
bind(&foo, _2, _1)('a', 5);
```

```cpp
struct A { int i; } a;
auto lambda = (_1->*&A::i) = _2;
lambda(a, 42);
```

```cpp
struct A { int foo(char, float); } a;
auto lambda = (_1->*&A::foo)(_2, 3.14f);
lambda(a, 'a');
```

License
-------

SPDX-License-Identifier: MIT

See [LICENSE](./LICENSE) for details.
