#pragma once

#include <clean-core/always_false.hh>
#include <clean-core/priority_tag.hh>
#include <clean-core/string.hh>
#include <clean-core/string_view.hh>
#include <clean-core/to_string.hh>

#include <reflector/introspect.hh>

namespace rf_external_detail
{
struct stringifier
{
    cc::string& s;

    int cnt = 0;

    template <class T>
    void operator()(T const& v, cc::string_view name);

    stringifier(cc::string& s) : s(s) { s += "{ "; }
    ~stringifier() { s += " }"; }
};

template <class T>
auto impl_to_string(T const& value, cc::priority_tag<2>) -> decltype(to_string(value))
{
    return to_string(value);
}

template <class T>
auto impl_to_string(T const& value, cc::priority_tag<1>) -> decltype(cc::to_string(value))
{
    return cc::to_string(value);
}

template <class T, class = std::enable_if_t<rf::is_introspectable<T>>>
cc::string impl_to_string(T const& value, cc::priority_tag<0>)
{
    cc::string str;
    {
        auto s = stringifier(str);
        rf::do_introspect(s, const_cast<T&>(value)); // promise we will not change anything!
    }
    return str;
}

template <class T, class = void>
struct has_to_string_t : std::false_type
{
};
template <class T>
struct has_to_string_t<T, std::void_t<decltype(::rf_external_detail::impl_to_string(std::declval<T>(), cc::priority_tag<2>{}))>> : std::true_type
{
};

template <class T>
void stringifier::operator()(T const& v, cc::string_view name)
{
    if (cnt > 0)
        s += ", ";

    s += name;
    s += ": ";

    if constexpr (std::is_convertible_v<T, cc::string>)
    {
        s += '"';
        s += v; // TODO: quote?
        s += '"';
    }
    else
    {
        static_assert(has_to_string_t<T>::value, "cannot stringify member");
        s += ::rf_external_detail::impl_to_string(v, cc::priority_tag<2>{});
    }

    ++cnt;
}
}
