#include <sstream>
#include "yurusanae.hpp"
#include "../keika.hpp"

using namespace keika;

Result<double> divide(double a, double b) {
    if (b == 0.0)
        return Result<double>::error("division by zero");
    else
        return a / b;
}

YURU_TEST(ctor_test) {
    assert_eq(Result<double>::ok(123).is_ok(), true);
    assert_eq(Result<double>::ok(123).is_error(), false);
    assert_eq(Result<double>::ok(123).ok(), 123.0);
    assert_eq(Result<double>{123}.ok(), 123.0);

    assert_eq(Result<double>::error("hoge").is_ok(), false);
    assert_eq(Result<double>::error("hoge").is_error(), true);
    assert_eq(Result<double>::error("hoge").error(), std::string{"hoge"});

    // copy/move ctor
    auto tmp = Result<int>::ok(42);
    assert_eq(tmp.ok(), 42);
    auto tmp2 = tmp;
    assert_eq(tmp2.ok(), 42);
    auto tmp3 = std::move(tmp);
    assert_eq(tmp3.ok(), 42);
    tmp = tmp2;
    assert_eq(tmp.ok(), 42);
    tmp = std::move(tmp3);
    assert_eq(tmp.ok(), 42);
}

YURU_TEST(print_test) {
    std::stringstream ss;
    ss << Result<int>::ok(123);
    assert_eq(ss.str(), std::string{"Result::ok(123)"});
    ss.str(std::string{});
    ss << Result<int>::error("hogefuga");
    assert_eq(ss.str(), std::string{"Result::error(hogefuga)"});

    ss.str(std::string{});
    ss << (Result<int>::ok(123)
        | keika::trying >> [](int i) {
            if (i%2 == 0)
                return i/2;
            else
                throw std::logic_error{"odd number!!"};
        });
    assert_eq(ss.str(), std::string{"Result::error(odd number!!)"});

    ss.str(std::string{});
    ss << Result<int>::trying([]() -> int {
            throw std::logic_error{"something happened!!"};
        });
    assert_eq(ss.str(), std::string{"Result::error(something happened!!)"});

}

YURU_TEST(logical_test) {
    // and
    assert_eq(divide(1.0, 1.0) && divide(1.0, 0.0), Result<double>::error("division by zero"));
    assert_eq(divide(1.0, 0.0) && divide(1.0, 1.0), Result<double>::error("division by zero"));
    assert_eq(divide(1.0, 1.0) && divide(1.0, 2.0), Result<double>::ok(0.5));

    // or
    assert_eq(divide(1.0, 1.0) || divide(1.0, 0.0), Result<double>::ok(1));
    assert_eq(divide(1.0, 0.0) || divide(1.0, 2.0), Result<double>::ok(0.5));
    assert_eq(divide(1.0, 2.0) || divide(1.0, 1.0), Result<double>::ok(0.5));
    assert_eq(divide(1.0, 0.0) || divide(2.0, 0.0), Result<double>::error("division by zero"));
}

YURU_TEST(operation_test) {
    auto res = divide(1.0, 1.0)    // Result::ok(1)
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
    assert_eq(res, Result<int>::ok(22));
}

YURU_TEST(match_test) {
    assert_eq(divide(1.0, 1.0).case_of(
        keika::ok >> [](double d) { return (int)d * 2; },
        keika::error >> [](std::string const& str) { return (int)str.size(); }
    ), 2);
    assert_eq(divide(1.0, 0.0).case_of(
        keika::ok >> [](double d) { return (int)d * 2; },
        keika::error >> [](std::string const& str) { return (int)str.size(); }
    ), 16);

}

int main() {
    ctor_test{};
    print_test{};
    logical_test{};
    operation_test{};
    match_test{};
}

