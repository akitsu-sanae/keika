# Keika

Keika is the error-handling library for C++, inspired by Rust's Result.  

# How to use

Keika is a header only library.  
All you must do to install is `#include "keika.hpp"`

# Examples

The simple example is here:

```cpp
using namespace keika;

Result<double> divide(double a, double b) {
    if (b == 0.0)
        return Result<double>::error("division by zero!!");
    else
        return a / b;
}

int main() {
    std::cout << divide(1.0, 1.0) << std::endl; // "Result::ok(1)"
    std::cout << divide(1.0, 0.0) << std::endl; // "Result::error(division by zero!!)"
}
```

You do not want to use `std::string` as error type? Ok
```cpp
using namespace keika;

// your custom error type
struct error_info {
    ...
};

Result<double, error_info> divide(double a, double b) {
    if (b == 0.0)
        return Result<double>::error(error_info{...});
    else
        return a / b;
}
```

Keika provides some operation for Result:

```cpp
int main() {
    divide(1.0, 1.0)    // Result::ok(1)
        | keika::map >> [](double d) { return std::to_string(d) + " is result!"; } // Result::ok(2 is result!)
        | keika::and_then >> [](std::string const& msg) {
            if (msg.size() > 10)
                return Result<int>::error("msg is too long!");
            else
                return Result<int>::ok(msg.size());
        } // Result::error(msg is too long)
        | keika::map_error >> [](std::string const& msg) {
            return msg + "nyan!!";
        } // Result::error(msg is too long!nyan!!)
        | keika::or_else >> [](std::string const& msg) {
            if (msg.size() > 10)
                return Result<int>::ok(msg.size());
            else
                return Result<int>::error("msg is too short!!");
        } // Result::ok(22)
        ;

    // and
    divide(1.0, 1.0) && divide(1.0, 0.0); // Result::error(division by zero)
    divide(1.0, 0.0) && divide(1.0, 1.0); // Result::error(division by zero)
    divide(1.0, 1.0) && divide(1.0, 2.0); // Result::ok(0.5)

    // or
    divide(1.0, 1.0) || divide(1.0, 0.0); // Result::ok(1)
    divide(1.0, 0.0) || divide(1.0, 2.0); // Result::ok(0.5)
    divide(1.0, 2.0) || divide(1.0, 1.0); // Result::ok(0.5)
    divide(1.0, 0.0) || divide(2.0, 0.0); // Result::error(division by zero)
}
```

You can also use this pattern-match like operation:
```cpp
int main() {
    divide(1.0, 1.0).case_of(
        keika::ok >> [](double d) { return (int)d * 2; },
        keika::error >> [](std::string const& str) { return (int)str.size(); }
    ); // 2

    divide(1.0, 0.0).case_of(
        keika::ok >> [](double d) { return (int)d * 2; },
        keika::error >> [](std::string const& str) { return (int)str.size(); }
    ); // 16

    divide(1.0, 0.0).case_of(
        keika::ok >> [](double d) { return d * 2.0; },
        keika::error >> [](std::string const& str) { return (int)str.size(); }
    ); // assertion!! "keika::Result::case_of: return types are different!" (double and int)
}
```

# Copyright
Copyright (C) 2017 akitsu sanae.  
Distributed under the Boost Software License, Version 1.0. 
(See accompanying file LICENSE or copy at http://www.boost/org/LICENSE_1_0.txt)  


