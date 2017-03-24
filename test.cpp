#include <iostream>
#include <string>
#include "keika.hpp"

using namespace keika;

auto divide(double a, double b) -> Result<double> {
    if (b == 0.0)
        return Result<double>::error("division by zero");
    else
        return a / b;
}

int main() {
    std::cout << (divide(1.0, 2.0)
        | keika::map >> [](double d) { return d * 2; }
        | keika::map >> [](double d) { return std::to_string(d);  }
        | keika::map >> [](std::string const& str) { return str + "hogefuga"; }) << std::endl;
    std::cout << (divide(1.0, 0.0)
        | keika::map >> [](double d) { return d * 2; }) << std::endl;

    std::cout << (divide(1.0, 0.0)
            | keika::map >> [](double d) { return d * 2; }
            | keika::map_error >> [] (std::string const&) { return "hoge"; }) << std::endl;

    std::cout << (divide(1.0, 1.0) && divide(1.0, 0.0)) << std::endl;

    std::cout << (divide(1.0, -1.0)
        | keika::and_then >> [](double d) {
            using ret_type = Result<double>;
            if (d < 0.0)
                return ret_type::error("less than zero");
            else
               return ret_type{d * 1.2};
        }) << std::endl;

    std::cout << (divide(1.0, 0.0) || divide(2.0, 1.0)) << std::endl;
    std::cout << (divide(1.0, 0.0)
        | keika::or_else >> [](std::string const& msg) {
            if (msg.size() < 10)
                return Result<double>(msg.size());
            else
                return Result<double>::error("error msg is too long");
        }) << std::endl;
}

