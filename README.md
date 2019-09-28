cpp-expression
==============

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
std::for_each(begin(v), end(v), variable(sum) += _1);
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
