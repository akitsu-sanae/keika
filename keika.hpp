/*============================================================================
  Copyright (C) 2017 akitsu sanae
  https://github.com/akitsu-sanae/keika
  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
============================================================================*/


#ifndef KEIKA_HPP
#define KEIKA_HPP

#include <stdexcept>
#include <exception>
#include <string>

namespace keika {

#define MAKE_ADAPTOR(X) \
    namespace detail { struct X ## _impl {}; } \
    static const detail:: X ## _impl X {}; \
    template<typename F> struct X ## _adaptor { F const& f; }; \
    template<typename F> inline static auto operator>>(detail:: X ## _impl const&, F const& f) { \
        return X ## _adaptor <F>{f}; \
    }

MAKE_ADAPTOR(ok);
MAKE_ADAPTOR(error);
#undef MAKE_ADAPTOR


template<typename T, typename E=std::string>
struct Result {
    using ok_t = T;
    using error_t = E;

    // constructions

    static Result ok(ok_t const& o) {
        Result result;
        result.m_is_ok = true;
        result.assign_ok(o);
        return result;
    }
    static Result ok(ok_t&& o) {
        Result result;
        result.m_is_ok = true;
        result.assign_ok(std::move(o));
        return result;
    }
    static Result error(error_t const& e) {
        Result result;
        result.m_is_ok = false;
        result.assign_err(e);
        return result;
    }
    static Result error(error_t&& e) {
        Result result;
        result.m_is_ok = false;
        result.assign_err(std::move(e));
        return result;
    }
    template<typename F>
    static Result<T, std::exception_ptr> trying(F const& f) {
        using ret_type = Result<T, std::exception_ptr>;
        try {
            return ret_type::ok(f());
        } catch (...) {
            return ret_type::error(std::current_exception());
        }
    }

    Result(ok_t const& ok) :
        m_is_ok{true}
    {
        assign_ok(ok);
    }
    Result(ok_t&& ok) :
        m_is_ok{true}
    {
        assign_ok(std::move(ok));
    }

    Result(Result const& src) :
        m_is_ok{src.m_is_ok}
    {
        if (m_is_ok)
            assign_ok(src.m_ok);
        else
            assign_err(src.m_error);
    }
    Result(Result&& src) :
        m_is_ok{src.m_is_ok}
    {
        if (m_is_ok)
            assign_ok(std::move(src.m_ok));
        else
            assign_err(std::move(src.m_error));
    }

    // copy
    Result& operator=(Result const& rhs) {
        if (m_is_ok == rhs.m_is_ok) {
            if (m_is_ok)
                m_ok = rhs.m_ok;
            else
                m_error = rhs.m_error;
        } else {
            clear();
            m_is_ok = rhs.m_is_ok;
            if (m_is_ok)
                assign_ok(rhs.m_ok);
            else
                assign_err(rhs.m_error);
        }
        return *this;
    }

    // move
    Result& operator=(Result&& rhs) {
        if (m_is_ok == rhs.m_is_ok) {
            if (m_is_ok)
                m_ok = std::move(rhs.m_ok);
            else
                m_error = std::move(rhs.m_error);
        } else {
            clear();
            m_is_ok = rhs.m_is_ok;
            if (m_is_ok)
                assign_ok(std::move(rhs.m_ok));
            else
                assign_err(std::move(rhs.m_error));
        }
        return *this;
    }

    ok_t const& ok() const {
        if (!m_is_ok)
            throw std::logic_error{"invalid keika::Result::ok for error result"};
        return m_ok;
    }
    error_t const& error() const {
        if (m_is_ok)
            throw std::logic_error{"invalid keika::Result::error for ok result"};
        return m_error;
    }

    bool is_ok() const { return m_is_ok; }
    bool is_error() const { return !m_is_ok; }

    template<typename F, typename G>
    auto case_of(ok_adaptor<F> const& ok_adap, error_adaptor<G> const& err_adap) const {
        using ok_ret_type = decltype(ok_adap.f(std::declval<ok_t>()));
        using err_ret_type = decltype(err_adap.f(std::declval<error_t>()));
        static_assert(std::is_same<ok_ret_type, err_ret_type>::value, "keika::Result::case_of: return types are different!");
        if (m_is_ok)
            return ok_adap.f(m_ok);
        else
            return err_adap.f(m_error);
    }

    ~Result() { clear(); }
private:
    Result() {}

    union {
        T m_ok;
        E m_error;
    };

    void clear() {
        if (m_is_ok)
            m_ok.~T();
        else
            m_error.~E();
    }

    void assign_ok(T&& ok) {
        new (&m_ok) T{std::move(ok)};
    }
    void assign_err(E&& err) {
        new (&m_error) E{std::move(err)};
    }

    void assign_ok(T const& ok) {
        new (&m_ok) T{ok};
    }
    void assign_err(E const& err) {
        new (&m_error) E{err};
    }

    bool m_is_ok;
};

template<typename T, typename U, typename E>
inline static Result<U, E> operator&&(Result<T, E> const& lhs, Result<U, E> const& rhs) {
    if (lhs.is_ok())
        return rhs;
    else
        return Result<U, E>::error(lhs.error());
}

template<typename T, typename U, typename E>
inline static Result<U, E> operator||(Result<T, E> const& lhs, Result<U, E> const& rhs) {
    if (lhs.is_ok())
        return Result<U, E>::ok(lhs.ok());
    else
        return rhs;
}

template<typename T, typename E>
inline static bool operator==(Result<T, E> const& lhs, Result<T, E> const& rhs) {
    if (lhs.is_ok() != rhs.is_ok())
        return false;
    if (lhs.is_ok())
        return lhs.ok() == rhs.ok();
    else
        return rhs.error() == rhs.error();
}

template<typename T, typename E>
inline static std::ostream&
operator <<(std::ostream& os, Result<T, E> const& res) {
    if (res.is_ok())
        os << "Result::ok(" << res.ok() << ")";
    else
        os << "Result::error(" << res.error() << ")";
    return os;
}

template<typename T>
inline static std::ostream&
operator <<(std::ostream& os, Result<T, std::exception_ptr> const& res) {
    if (res.is_ok()) {
        os << "Result::ok(" << res.ok() << ")";
        return os;
    }
    try {
        std::rethrow_exception(res.error());
    } catch (std::exception const& e) {
        os << "Result::error(" << e.what() << ")";
    } catch (...) {
        os << "Result::error(non standard exception)";
    }
    return os;
}

#define MAKE_ADAPTOR(X) \
    namespace detail { struct X ## _impl {}; } \
    static const detail:: X ## _impl X {}; \
    template<typename F> struct X ## _adaptor { F const& f; }; \
    template<typename F> inline static auto operator>>(detail:: X ## _impl const&, F const& f) { \
        return X ## _adaptor <F>{f}; \
    }

MAKE_ADAPTOR(map);
MAKE_ADAPTOR(map_error);
MAKE_ADAPTOR(and_then);
MAKE_ADAPTOR(or_else);
MAKE_ADAPTOR(trying);
#undef MAKE_ADAPTOR

template<typename T, typename E, typename F>
auto operator|(Result<T, E> result, map_adaptor<F> const& adaptor) {
    using ret_type = Result<decltype(adaptor.f(std::declval<T>())), E>;
    if (result.is_ok())
        return ret_type::ok(adaptor.f(result.ok()));
    else
        return ret_type::error(result.error());
}

template<typename T, typename E, typename F>
auto operator|(Result<T, E> result, map_error_adaptor<F> const& adaptor) {
    using ret_type = Result<T, decltype(adaptor.f(std::declval<E>()))>;
    if (result.is_error())
        return ret_type::error(adaptor.f(result.error()));
    else
        return ret_type::ok(result.ok());
}
template<typename T, typename E, typename F>
auto operator|(Result<T, E> result, and_then_adaptor<F> const& adaptor) {
    using ret_type = decltype(adaptor.f(std::declval<T>()));
    if (result.is_ok())
        return adaptor.f(result.ok());
    else
        return ret_type::error(result.error());
}
template<typename T, typename E, typename F>
auto operator|(Result<T, E> result, or_else_adaptor<F> const& adaptor) {
    using ret_type = decltype(adaptor.f(std::declval<E>()));
    if (result.is_error())
        return adaptor.f(result.error());
    else
        return ret_type::ok(result.ok());
}
template<typename T, typename E, typename F>
auto operator|(Result<T, E> result, trying_adaptor<F> const& adaptor) {
    using ret_type = Result<decltype(adaptor.f(std::declval<T>())), std::exception_ptr>;
    if (result.is_error())
        return ret_type::error(std::make_exception_ptr(result.error()));

    try {
        return ret_type::ok(adaptor.f(result.ok()));
    } catch (...) {
        return ret_type::error(std::current_exception());
    }
}

}

#endif

