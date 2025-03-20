#ifndef CLOPTS_H
#define CLOPTS_H

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#ifndef CLOPTS_USE_MMAP
#    ifdef __linux__
#        define CLOPTS_USE_MMAP 1
#    else
#        define CLOPTS_USE_MMAP 0
#    endif
#endif

#if CLOPTS_USE_MMAP
#    include <fcntl.h>
#    include <sys/mman.h>
#    include <sys/stat.h>
#    include <unistd.h>
#else
#    include <fstream>
#endif

/// \brief Main library namespace.
///
/// The name of this is purposefully verbose to avoid name collisions. Users are
/// recommended to use a namespace alias instead.
namespace command_line_options {
/// ===========================================================================
///  Internals.
/// ===========================================================================
namespace detail { // clang-format off
/// Some compilers do not have __builtin_strlen().
#if defined __GNUC__ || defined __clang__
#    define CLOPTS_STRLEN(str)  __builtin_strlen(str)
#    define CLOPTS_STRCMP(a, b) __builtin_strcmp(a, b)
#else
constexpr inline std::size_t CLOPTS_STRLEN(const char* str) {
    std::size_t len = 0;
    while (*str++) ++len;
    return len;
}

constexpr inline int CLOPTS_STRCMP(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        ++a;
        ++b;
    }
    return *a - *b;
}

#    define CLOPTS_STRLEN(str)  ::command_line_options::detail::CLOPTS_STRLEN(str)
#    define CLOPTS_STRCMP(a, b) ::command_line_options::detail::CLOPTS_STRCMP(a, b)
#endif

/// Raise a compile-time error.
#ifndef CLOPTS_ERR
#    define CLOPTS_ERR(msg) [] <bool _x = false> { static_assert(_x, msg); } ()
#endif

/// Constexpr to_string for integers. Returns the number of bytes written.
constexpr std::size_t constexpr_to_string(char* out, std::int64_t i) {
    /// Special handling for 0.
    if (i == 0) {
        *out = '0';
        return 1;
    }

    const auto start = out;
    if (i < 0) {
        *out++ = '-';
        i = -i;
    }

    while (i) {
        *out++ = char('0' + char(i % 10));
        i /= 10;
    }

    std::reverse(start, out);
    return std::size_t(out - start);
}

/// Check if two types are the same.
template <typename a, typename ...bs>
concept is = (std::is_same_v<std::remove_cvref_t<a>, std::remove_cvref_t<bs>> or ...);

/// Check if two types are exactly the same.
template <typename a, typename ...bs>
concept is_same = (std::is_same_v<a, bs> or ...);

/// Check if an operand type is a vector.
template <typename t> struct test_vector;
template <typename t> struct test_vector<std::vector<t>> {
    static constexpr bool value = true;
    using type = t;
};

template <typename t> struct test_vector {
    static constexpr bool value = false;
    using type = t;
};

template <typename t> concept is_vector_v = test_vector<t>::value;
template <typename t> using remove_vector_t = typename test_vector<t>::type;

/// Get the base type of an option.
template <typename t> struct base_type_s;
template <typename t> struct base_type_s<std::vector<t>> { using type = t; };
template <typename t> struct base_type_s { using type = t; };
template <typename t> using base_type_t = typename base_type_s<t>::type;

/// Check if an option is a positional option.
template <typename opt>
struct is_positional {
    static constexpr bool value = requires {{typename opt::is_positional_{}} -> std::same_as<std::true_type>; };
    using type = std::bool_constant<value>;
};

template <typename opt> using positional_t = typename is_positional<opt>::type;
template <typename opt> concept is_positional_v = is_positional<opt>::value;

/// Callback that takes an argument.
using callback_arg_type = void (*)(void*, std::string_view, std::string_view);

/// Callback that takes no arguments.
using callback_noarg_type = void (*)(void*, std::string_view);

/// Check whether a type is a callback.
template <typename T>
concept is_callback = is<T,
    callback_arg_type,
    callback_noarg_type,
    std::vector<detail::callback_arg_type>,
    std::vector<detail::callback_noarg_type>
>;

/// Check if an option type takes an argument.
template <typename type>
concept has_argument = not is<type, bool, callback_noarg_type>;

/// Whether we should include the argument type of an option in the
/// help text. This is true for all options that take arguments, except
/// the builtin help option.
template <typename opt>
concept should_print_argument_type = has_argument<typename opt::type> and not requires { opt::is_help_option; };

/// Helper for static asserts.
template <typename t>
concept always_false = false;

/// Not a concept because we can’t pass packs as the first parameter of a concept.
template <typename first, typename ...rest>
static constexpr bool assert_same_type = (std::is_same_v<first, rest> and ...);

/// Wrap an arbitrary function in a lambda.
template <auto cb> struct make_lambda_s;

template <auto cb>
requires std::is_invocable_v<decltype(cb)>
struct make_lambda_s<cb> {
    using lambda = decltype([](void*, std::string_view) { cb(); });
    using type = callback_noarg_type;
};

template <auto cb>
requires std::is_invocable_v<decltype(cb), void*>
struct make_lambda_s<cb> {
    using lambda = decltype([](void* data, std::string_view) { cb(data); });
    using type = callback_noarg_type;
};

template <auto cb>
requires std::is_invocable_v<decltype(cb), std::string_view>
struct make_lambda_s<cb> {
    using lambda = decltype([](void*, std::string_view, std::string_view arg) { cb(arg); });
    using type = callback_arg_type;
};

template <auto cb>
requires std::is_invocable_v<decltype(cb), void*, std::string_view>
struct make_lambda_s<cb> {
    using lambda = decltype([](void* data, std::string_view, std::string_view arg) { cb(data, arg); });
    using type = callback_arg_type;
};

template <auto cb>
requires std::is_invocable_v<decltype(cb), std::string_view, std::string_view>
struct make_lambda_s<cb> {
    using lambda = decltype([](void*, std::string_view name, std::string_view arg) { cb(name, arg); });
    using type = callback_arg_type;
};

template <auto cb>
requires std::is_invocable_v<decltype(cb), void*, std::string_view, std::string_view>
struct make_lambda_s<cb> {
    using lambda = decltype([](void* data, std::string_view name, std::string_view arg) { cb(data, name, arg); });
    using type = callback_arg_type;
};

template <auto cb>
using make_lambda = make_lambda_s<cb>; // clang-format on

template <typename first, typename... rest>
struct first_type {
    using type = first;
};

/// Get the first element of a pack.
template <typename... rest>
using first_type_t = typename first_type<rest...>::type;

/// Execute code at end of scope.
template <typename lambda>
struct at_scope_exit {
    lambda l;
    ~at_scope_exit() { l(); }
};

/// Tag used for options that modify the options (parser) but
/// do not constitute actual options in an of themselves.
struct noop_tag {};

/// Compile-time string.
template <size_t sz>
struct static_string {
    char arr[sz]{};
    size_t len{};

    /// Construct an empty string.
    constexpr static_string() {}

    /// Construct from a string literal.
    constexpr static_string(const char (&_data)[sz]) {
        std::copy_n(_data, sz, arr);
        len = sz - 1;
    }

    /// Check if two strings are equal.
    template <typename str>
    requires requires { std::declval<str>().len; }
    [[nodiscard]] constexpr bool operator==(const str& s) const {
        return len == s.len && CLOPTS_STRCMP(arr, s.arr) == 0;
    }

    /// Check if this is equal to a string.
    [[nodiscard]] constexpr bool operator==(std::string_view s) const {
        return sv() == s;
    }

    /// Append to this string.
    template <size_t n>
    constexpr void operator+=(const static_string<n>& str) {
        static_assert(len + str.len < sz, "Cannot append string because it is too long");
        std::copy_n(str.arr, str.len, arr + len);
        len += str.len;
    }

    /// Append a string literal to this string.
    constexpr void append(const char* str) { append(str, CLOPTS_STRLEN(str)); }

    /// Append a string literal with a known length to this string.
    constexpr void append(const char* str, size_t length) {
        std::copy_n(str, length, arr + len);
        len += length;
    }

    /// Get the string as a \c std::string_view.
    [[nodiscard]] constexpr auto sv() const -> std::string_view { return {arr, len}; }

    /// API for static_assert.
    [[nodiscard]] constexpr auto data() const -> const char* { return arr; }
    [[nodiscard]] constexpr auto size() const -> std::size_t { return len; }

    static constexpr bool is_static_string = true;
};

/// Deduction guide to shut up nonsense CTAD warnings.
template <size_t sz>
static_string(const char (&)[sz]) -> static_string<sz>;

template <std::size_t size>
struct string_or_int {
    detail::static_string<size> s{};
    std::int64_t integer{};
    bool is_integer{};

    constexpr string_or_int(const char (&data)[size]) {
        std::copy_n(data, size, s.arr);
        s.len = size - 1;
        is_integer = false;
    }

    constexpr string_or_int(std::int64_t integer)
        : integer{integer}
        , is_integer{true} {}
};

string_or_int(std::int64_t) -> string_or_int<1>;

/// Struct for storing allowed option values.
template <typename _type, auto... data>
struct values_impl {
    using type = _type;
    constexpr values_impl() = delete;

    static constexpr bool is_valid_option_value(const type& val) {
        auto test = [val]<auto value>() -> bool {
            if constexpr (value.is_integer) return value.integer == val;
            else return value.s == val;
        };

        return (test.template operator()<data>() or ...);
    }

    static constexpr auto print_values(char* out) -> std::size_t {
        /// TODO: Wrap and indent every 10 or so values?
        bool first = true;
        auto append = [&]<auto value>() -> std::size_t {
            if (first) first = false;
            else {
                std::copy_n(", ", 2, out);
                out += 2;
            }
            if constexpr (value.is_integer) {
                char s[32]{};
                auto len = constexpr_to_string(s, value.integer);
                std::copy_n(s, len, out);
                out += len;
                return len;
            } else {
                std::copy_n(value.s.arr, value.s.len, out);
                out += value.s.len;
                return value.s.len;
            }
        };
        return (append.template operator()<data>() + ...) + (sizeof...(data) - 1) * 2;
    };
};

template <string_or_int... data>
concept values_must_be_all_strings_or_all_ints = (data.is_integer and ...) or (not data.is_integer and ...);

/// Values type.
template <string_or_int... data>
requires values_must_be_all_strings_or_all_ints<data...>
struct values : values_impl<std::conditional_t<(data.is_integer and ...), std::int64_t, std::string>, data...> {};

/// Check that an option type is valid.
template <typename type>
concept is_valid_option_type = is_same<type, std::string, // clang-format off
    bool,
    double,
    int64_t,
    noop_tag,
    callback_arg_type,
    callback_noarg_type
> or is_vector_v<type> or requires { type::is_values; } or requires { type::is_file_data; };
// clang-format on

template <typename _type>
struct option_type {
    using type = _type;
    static constexpr bool is_values = false;
};

/// Look through values<> to figure out the option type.
template <auto... vs>
struct option_type<values<vs...>> {
    using type = values<vs...>::type;
    static constexpr bool is_values = true;
};

template <typename _type>
using option_type_t = typename option_type<_type>::type;

template <typename _type>
concept is_values_type_t = option_type<_type>::is_values;

template <
    static_string _name,
    static_string _description,
    typename ty_param,
    bool required>
struct opt_impl {
    /// There are four possible cases we need to handle here:
    ///   - Simple type: std::string, int64_t, ...
    ///   - Vector of simple type: std::vector<std::string>, std::vector<int64_t>, ...
    ///   - Values type: values<...>
    ///   - Vector of values type: std::vector<values<...>>

    /// The actual type that was passed in.
    using actual_type = ty_param;

    /// The type stripped of top-level std::vector<>.
    using actual_type_base = remove_vector_t<actual_type>;

    /// The underlying simple type used to store one element.
    using simple_type = option_type_t<actual_type_base>;

    /// The type used to store the result of the option. This is either
    /// the simple type or a std::vector<> thereof.
    using type = std::conditional_t<is_vector_v<actual_type>, std::vector<simple_type>, simple_type>;

    /// Make sure this is a valid option.
    static_assert(sizeof _description.arr < 512, "Description may not be longer than 512 characters");
    static_assert(_name.len > 0, "Option name may not be empty");
    static_assert(sizeof _name.arr < 256, "Option name may not be longer than 256 characters");
    static_assert(not std::is_void_v<type>, "Option type may not be void. Use bool instead");
    static_assert(
        is_valid_option_type<type>,
        "Option type must be std::string, bool, int64_t, double, file_data, values<>, or callback"
    );

    static constexpr inline decltype(_name) name = _name;
    static constexpr inline decltype(_description) description = _description;
    static constexpr inline bool is_flag = std::is_same_v<type, bool>;
    static constexpr inline bool is_values = is_values_type_t<actual_type_base>;
    static constexpr inline bool is_required = required;
    static constexpr inline bool option_tag = true;

    static constexpr bool is_valid_option_value(
        const simple_type& val
    ) {
        if constexpr (is_values) return actual_type_base::is_valid_option_value(val);
        else return true;
    }

    static constexpr auto print_values(char* out) -> std::size_t {
        if constexpr (is_values) return actual_type_base::print_values(out);
        else return 0;
    }

    constexpr opt_impl() = delete;
};

/// Default help handler.
inline void default_help_handler(std::string_view program_name, std::string_view msg) {
    std::cerr << "Usage: " << program_name << " " << msg;
    std::exit(1);
}

template <typename file_data_type>
static file_data_type map_file(
    std::string_view path,
    auto error_handler = [](std::string&& msg) { std::cerr << msg << "\n"; std::exit(1); }
) {
    const auto err = [&](std::string_view p) -> file_data_type {
        std::string msg = "Could not read file \"";
        msg += p;
        msg += "\": ";
        msg += ::strerror(errno);
        error_handler(std::move(msg));
        return {};
    };

#if CLOPTS_USE_MMAP
    int fd = ::open(path.data(), O_RDONLY);
    if (fd < 0) return err(path);

    struct stat s {};
    if (::fstat(fd, &s)) return err(path);
    auto sz = size_t(s.st_size);
    if (sz == 0) return {};

    auto* mem = (char*) ::mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (mem == MAP_FAILED) return err(path);
    ::close(fd);

    /// Construct the file contents.
    typename file_data_type::contents_type ret;
    auto pointer = reinterpret_cast<typename file_data_type::element_pointer>(mem);
    if constexpr (requires { ret.assign(pointer, sz); }) ret.assign(pointer, sz);
    else if constexpr (requires { ret.assign(pointer, pointer + sz); }) ret.assign(pointer, pointer + sz);
    else CLOPTS_ERR("file_data_type::contents_type must have a callable assign member that takes a pointer and a size_t (or a begin and end iterator) as arguments.");
    ::munmap(mem, sz);

#else
    using contents_type = typename file_data_type::contents_type;

    /// Read the file manually.
    std::unique_ptr<FILE, decltype(&std::fclose)> f{std::fopen(path.data(), "rb"), std::fclose};
    if (not f) return err(path);

    /// Get the file size.
    std::fseek(f.get(), 0, SEEK_END);
    auto sz = std::size_t(std::ftell(f.get()));
    std::fseek(f.get(), 0, SEEK_SET);

    /// Read the file.
    contents_type ret;
    ret.resize(sz);
    std::size_t n_read = 0;
    while (n_read < sz) {
        auto n = std::fread(ret.data() + n_read, 1, sz - n_read, f.get());
        if (n < 0) return err(path);
        if (n == 0) break;
        n_read += n;
    }
#endif

    /// Construct the file data.
    file_data_type dat;
    dat.path = typename file_data_type::path_type{path.begin(), path.end()};
    dat.contents = std::move(ret);
    return dat;
}

/// ===========================================================================
///  Helper functions to process parameters packs.
/// ===========================================================================
/// Iterate over a pack while a condition is true.
template <typename... pack>
constexpr void Foreach(auto&& lambda) {
    (lambda.template operator()<pack>(), ...);
}

/// Iterate over a pack while a condition is true.
template <typename... pack>
constexpr void While(bool& cond, auto&& lambda) {
    auto impl = [&]<typename t>() -> bool {
        if (not cond) return false;
        else {
            lambda.template operator()<t>();
            return true;
        }
    };

    (impl.template operator()<pack>() and ...);
}

/// ===========================================================================
///  Main implementation.
/// ===========================================================================
template <typename... opts>
class clopts_impl {
    using This = clopts_impl<opts...>;

    /// This should never be instantiated by the user.
    explicit clopts_impl() {}
    ~clopts_impl() {}
    clopts_impl(const clopts_impl& o) = delete;
    clopts_impl(clopts_impl&& o) = delete;
    clopts_impl& operator=(const clopts_impl& o) = delete;
    clopts_impl& operator=(clopts_impl&& o) = delete;

    /// =======================================================================
    ///  Validation.
    /// =======================================================================
    /// Make sure no two options have the same name.
    static consteval bool check_duplicate_options() {
        /// State is ok initially.
        bool ok = true;
        std::size_t i = 0;

        /// Iterate over each option for each option.
        While<opts...>(ok, [&]<typename opt>() {
            std::size_t j = 0;
            While<opts...>(ok, [&]<typename opt2>() {
                /// If the options are not the same, but their names are the same
                /// then this is an error. Iteration will stop at this point because
                /// \c ok is also the condition for the two loops.
                ok = i == j or opt::name != opt2::name;
                j++;
            });
            i++;
        });

        /// Return whether everything is ok.
        return ok;
    }

    /// Make sure that no option has a prefix that is a short option.
    static consteval bool check_short_opts() {
        /// State is ok initially.
        bool ok = true;
        std::size_t i = 0;

        /// Iterate over each option for each option.
        While<opts...>(ok, [&]<typename opt>() {
            std::size_t j = 0;
            While<opts...>(ok, [&]<typename opt2>() {
                /// Check the condition.
                ok = i == j or not requires { opt::is_short; } or not opt2::name.sv().starts_with(opt::name.sv());
                j++;
            });
            i++;
        });

        /// Return whether everything is ok.
        return ok;
    }

    /// Make sure there is at most one multiple<positional<>> option.
    static consteval size_t validate_multiple() {
        auto is_mul = []<typename opt>() { return requires { opt::is_multiple; }; };
        return (... + (is_mul.template operator()<opts>() and detail::is_positional_v<opts>) );
    }

    /// Make sure we don’t have invalid option combinations.
    static_assert(check_duplicate_options(), "Two different options may not have the same name");
    static_assert(check_short_opts(), "Option name may not start with the name of a short option");
    static_assert(validate_multiple() <= 1, "Cannot have more than one multiple<positional<>> option");

    /// Various types.
    using help_string_t = detail::static_string<1024 * sizeof...(opts)>;
    using optvals_tuple_t = std::tuple<typename opts::type...>;
    using string = std::string;
    using integer = int64_t;

public:
    using error_handler_t = std::function<bool(std::string&&)>;

private:
    /// =======================================================================
    ///  Option access.
    /// =======================================================================
    /// Get the name of an option type.
    template <typename t>
    static consteval auto type_name() -> detail::static_string<25> {
        detail::static_string<25> buffer;
        if constexpr (detail::is<t, string>) buffer.append("string");
        else if constexpr (detail::is<t, bool>) buffer.append("bool");
        else if constexpr (detail::is<t, integer, double>) buffer.append("number");
        else if constexpr (requires { t::is_file_data; }) buffer.append("file");
        else if constexpr (detail::is_callback<t>) buffer.append("arg");
        else if constexpr (detail::is_vector_v<t>) {
            buffer.append(type_name<typename t::value_type>().arr, type_name<typename t::value_type>().len);
            buffer.append("s");
        } else {
            CLOPTS_ERR("Option type must be std::string, bool, integer, double, or void(*)(), or a vector thereof");
        }
        return buffer;
    }

    /// Get the index of an option.
    template <size_t index, detail::static_string option>
    static constexpr size_t optindex_impl() {
        if constexpr (index >= sizeof...(opts)) return index;
        else if constexpr (CLOPTS_STRCMP(opt_names[index], option.arr) == 0) return index;
        else return optindex_impl<index + 1, option>();
    }

#if __cpp_static_assert >= 202306L
    template <detail::static_string option>
    static consteval auto format_invalid_option_name() -> detail::static_string<option.size() + 45> {
        detail::static_string<option.size() + 45> ret;
        ret.append("There is no option with the name '");
        ret.append(option.data(), option.size());
        ret.append("'");
        return ret;
    }
#endif

    template <bool ok, detail::static_string option>
    static consteval void assert_valid_option_name() {
#if __cpp_static_assert >= 202306L
        static_assert(ok, format_invalid_option_name<option>());
#else
        static_assert(ok, "Invalid option name. You've probably misspelt an option.");
#endif
    }

    /// Get the index of an option and raise an error if the option is not found.
    template <detail::static_string option>
    static constexpr size_t optindex() {
        constexpr size_t sz = optindex_impl<0, option>();
        assert_valid_option_name<(sz < sizeof...(opts)), option>();
        return sz;
    }

    /// Get the type of an option value.
    template <detail::static_string s>
    using optval_t = std::remove_cvref_t<decltype(std::get<optindex<s>()>(std::declval<optvals_tuple_t>()))>;

public:
    /// Result type.
    class optvals_type {
        friend clopts_impl;
        optvals_tuple_t optvals{};
        std::array<bool, sizeof...(opts)> opts_found{};

        /// This implements get<>() and get_or<>().
        template <detail::static_string s>
        constexpr auto get_impl() -> std::conditional_t<std::is_same_v<optval_t<s>, bool>, bool, optval_t<s>*> {
            using value_type = optval_t<s>;

            /// Bool options don’t have a value. Instead, we just return whether the option was found.
            if constexpr (std::is_same_v<value_type, bool>) return opts_found[optindex<s>()];

            /// We always return a pointer to vector options because the user can just check if it’s empty.
            else if constexpr (detail::is_vector_v<value_type>) return std::addressof(std::get<optindex<s>()>(optvals));

            /// Function options don’t have a value.
            else if constexpr (detail::is_callback<value_type>) CLOPTS_ERR("Cannot call get<>() on an option with function type.");

            /// Otherwise, return nullptr if the option wasn’t found, and a pointer to the value otherwise.
            else return not opts_found[optindex<s>()] ? nullptr : std::addressof(std::get<optindex<s>()>(optvals));
        }

    public:
        /// \brief Get the value of an option.
        ///
        /// This is not \c [[nodiscard]] because that raises an ICE when compiling
        /// with some older versions of GCC.
        ///
        /// \return \c true / \c false if the option is a flag
        /// \return \c nullptr if the option was not found
        /// \return a pointer to the value if the option was found
        ///
        /// \see get_or()
        template <detail::static_string s>
        constexpr auto get() {
            /// Check if the option exists before calling get_impl<>() so we trigger the static_assert
            /// below before hitting a complex template instantiation error.
            constexpr auto sz = optindex_impl<0, s>();
            if constexpr (sz < sizeof...(opts)) return get_impl<s>();
            else assert_valid_option_name<(sz < sizeof...(opts)), s>();
        }

        /// \brief Get the value of an option or a default value if the option was not found.
        ///
        /// The default value is \c static_cast to the type of the option value.
        ///
        /// \param default_ The default value to return if the option was not found.
        /// \return \c default_ if the option was not found.
        /// \return a copy of the option value if the option was found.
        ///
        /// \see get()
        template <detail::static_string s>
        constexpr auto get_or(auto default_) {
            constexpr auto sz = optindex_impl<0, s>();
            if constexpr (sz < sizeof...(opts)) {
                if (opts_found[optindex<s>()]) return *get_impl<s>();
                else return static_cast<std::remove_cvref_t<decltype(*get_impl<s>())>>(default_);
            } else {
                assert_valid_option_name<(sz < sizeof...(opts)), s>();
            }
        }
    };

private:
    /// Option names.
    static constexpr inline std::array<const char*, sizeof...(opts)> opt_names{opts::name.arr...};

    /// Variables for the parser and for storing parsed options.
    optvals_type optvals{};
    bool has_error = false;
    int argc{};
    int argi{};
    const char** argv{};
    void* user_data{};
    error_handler_t error_handler{};

    /// Check if an option was found.
    template <detail::static_string option>
    bool found() { return optvals.opts_found[optindex<option>()]; }

    /// Mark an option as found.
    template <detail::static_string option>
    void set_found() { optvals.opts_found[optindex<option>()] = true; }

    /// Get a reference to an option value.
    template <detail::static_string s>
    [[nodiscard]] constexpr auto ref() -> decltype(std::get<optindex<s>()>(optvals.optvals))& {
        using value_type = decltype(std::get<optindex<s>()>(optvals.optvals));

        /// Bool options don’t have a value.
        if constexpr (std::is_same_v<value_type, bool>) CLOPTS_ERR("Cannot call ref() on an option<bool>");

        /// Function options don’t have a value.
        else if constexpr (detail::is_callback<value_type>) CLOPTS_ERR("Cannot call ref<>() on an option with function type.");

        /// Get the option value.
        else return std::get<optindex<s>()>(optvals.optvals);
    }

    /// Create the help message.
    static constexpr auto make_help_message() -> help_string_t { // clang-format off
        help_string_t msg{};

        /// Append the positional options.
        Foreach<opts...>([&]<typename opt> () {
            if constexpr (detail::is_positional_v<opt>) {
                msg.append("<");
                msg.append(opt::name.arr, opt::name.len);
                msg.append("> ");
            }
        });

        /// End of first line.
        msg.append("[options]\n");

        /// Start of options list.
        msg.append("Options:\n");

        /// Determine the length of the longest name + typename so that
        /// we know how much padding to insert before actually printing
        /// the description. Also factor in the <> signs around and the
        /// space after the option name, as well as the type name.
        size_t max_vals_opt_name_len{};
        size_t max_len{};
        auto determine_length = [&]<typename opt> {
            /// Positional options go on the first line, so ignore them here.
            if constexpr (not detail::is_positional_v<opt>) {
                if constexpr (opt::is_values)
                    max_vals_opt_name_len = std::max(max_vals_opt_name_len, opt::name.len);
                if constexpr (detail::should_print_argument_type<opt>) {
                    auto n = type_name<typename opt::type>();
                    max_len = std::max(max_len, opt::name.len + (n.len + sizeof("<> ") - 1));
                } else {
                    max_len = std::max(max_len, opt::name.len);
                }
            }
        };
        (determine_length.template operator()<opts>(), ...);

        /// Append the options
        auto append = [&] <typename opt> {
            /// Positional options have already been handled.
            if constexpr (not detail::is_positional_v<opt>) {
                /// Append the name.
                msg.append("    ");
                msg.append(opt::name.arr, opt::name.len);

                /// Compute the padding for this option and append the type name.
                size_t len = opt::name.len;
                if constexpr (detail::should_print_argument_type<opt>) {
                    const auto tname = type_name<typename opt::type>();
                    len += (3 + tname.len);
                    msg.append(" <");
                    msg.append(tname.arr, tname.len);
                    msg.append(">");
                }

                /// Append the padding.
                for (size_t i = 0; i < max_len - len; i++) msg.append(" ");

                /// Append the description.
                msg.append("  ");
                msg.append(opt::description.arr, opt::description.len);
                msg.append("\n");
            }
        };
        (append.template operator()<opts>(), ...);

        /// If we have any values<> types, print their supported values.
        if constexpr ((opts::is_values or ...)) {
            msg.append("\nSupported option values:\n");
            auto write_opt_vals = [&] <typename opt> () {
                if constexpr (opt::is_values) {
                    msg.append("    ");
                    msg.append(opt::name.arr, opt::name.len);
                    msg.append(":");

                    /// Padding after the name.
                    for (size_t i = 0; i < max_vals_opt_name_len - opt::name.len + 1; i++)
                        msg.append(" ");

                    /// Option values.
                    msg.len += opt::print_values(msg.arr + msg.len);
                    msg.append("\n");
                }
            };
            (write_opt_vals.template operator()<opts>(), ...);
        }


        /// Return the combined help message.
        return msg;
    }; // clang-format on

    /// Help message is created at compile time.
    static constexpr inline help_string_t help_message_raw = make_help_message();

public:
    /// Get the help message.
    static auto help() -> std::string {
        return {help_message_raw.arr, help_message_raw.len};
    }

private:
    /// Handle an option value.
    template <typename opt, bool is_multiple>
    auto dispatch_option_with_arg(std::string_view opt_str, std::string_view opt_val) {
        using opt_type = typename opt::type;

        /// Mark the option as found.
        set_found<opt::name>();

        /// If this is a function option, simply call the callback and we're done.
        if constexpr (detail::is_callback<opt_type>) {
            if constexpr (detail::is<opt_type, detail::callback_noarg_type>) opt::callback(user_data, opt_str);
            else opt::callback(user_data, opt_str, opt_val);
        }

        /// Otherwise, parse the argument.
        else {
            /// Create the argument value.
            auto value = make_arg<opt_type>(opt_val);

            /// If this option takes a list of values, check that the
            /// value matches one of them.
            if constexpr (opt::is_values) {
                if (not opt::is_valid_option_value(value)) {
                    handle_error(
                        "Invalid value for option '",
                        std::string(opt_str),
                        "': '",
                        std::string(opt_val),
                        "'"
                    );
                }
            }

            /// Set the value.
            if constexpr (is_multiple) ref<opt::name>().push_back(std::move(value));
            else ref<opt::name>() = std::move(value);
        }
    }

    /// Invoke the help callback of the help option.
    template <typename opt>
    void invoke_help_callback() {
        /// New API: program name + help message [+ user data].
        using sv = std::string_view;
        if constexpr (requires { opt::help_callback(sv{}, sv{}, user_data); })
            opt::help_callback(sv{argv[0]}, sv{}, user_data);
        else if constexpr (requires { opt::help_callback(sv{}, sv{}); })
            opt::help_callback(sv{argv[0]}, help_message_raw.sv());

        /// Compatibility for callbacks that don’t take the program name.
        else if constexpr (requires { opt::help_callback(sv{}, user_data); })
            opt::help_callback(help_message_raw.sv(), user_data);
        else if constexpr (requires { opt::help_callback(sv{}); })
            opt::help_callback(help_message_raw.sv());

        /// Invalid help option callback.
        else static_assert(
            detail::always_false<opt>,
            "Invalid help option signature. Consult the README for more information"
        );
    }

    /// Error handler that is used if the user doesn’t provide one.
    bool default_error_handler(std::string&& errmsg) {
        std::cerr << argv[0] << ": " << errmsg << "\n";

        /// Invoke the help option.
        bool invoked = false;
        auto invoke = [&]<typename opt> {
            if constexpr (requires { opt::is_help_option; }) {
                invoked = true;
                invoke_help_callback<opt>();
            }
        };

        /// If there is a help option, invoke it.
        (invoke.template operator()<opts>(), ...);

        /// If no help option was found, print the help message.
        if (not invoked) std::cerr << "Usage: " << argv[0] << " " << help();
        std::exit(1);
    };

    /// Invoke the error handler and set the error flag.
    void handle_error(auto first, auto&&... msg_parts) {
        /// Append the message parts.
        std::string msg = std::string{std::move(first)};
        ((msg += std::forward<decltype(msg_parts)>(msg_parts)), ...);

        /// Dispatch the error.
        has_error = not error_handler(std::move(msg));
    }

    /// Helper to parse an integer or double.
    template <typename number_type, detail::static_string name>
    auto parse_number(std::string_view s, auto parse_func) -> number_type {
        number_type i{};

        /// The empty string is a valid integer *and* float, apparently.
        if (s.empty()) {
            handle_error("Expected ", name.sv(), ", got empty string");
            return i;
        }

        /// Parse the number.
        errno = 0;
        char* pos{};
        if constexpr (requires { parse_func(s.data(), &pos, 10); }) i = number_type(parse_func(s.data(), &pos, 10));
        else i = number_type(parse_func(s.data(), &pos));
        if (errno != 0 or *pos) handle_error(s, " does not appear to be a valid ", name.sv());
        return i;
    }

    /// Parse an option value.
    template <typename type>
    detail::base_type_t<type> make_arg(std::string_view opt_val) {
        using base_type = detail::base_type_t<type>;

        /// Make sure this option takes an argument.
        if constexpr (not detail::has_argument<base_type>) CLOPTS_ERR("This option type does not take an argument");

        /// Strings do not require parsing.
        else if constexpr (std::is_same_v<base_type, std::string>) return std::string{opt_val};

        /// If it’s a file, read its contents.
        else if constexpr (requires { base_type::is_file_data; }) return detail::map_file<base_type>(opt_val, error_handler);

        /// Parse an integer or double.
        else if constexpr (std::is_same_v<base_type, integer>) return parse_number<integer, "integer">(opt_val, std::strtoull);
        else if constexpr (std::is_same_v<base_type, double>) return parse_number<double, "floating-point number">(opt_val, std::strtod);

        /// Should never get here.
        else CLOPTS_ERR("Unreachable");
    }

    /// Handle an option that may take an argument.
    ///
    /// Both --option value and --option=value are valid ways of supplying a
    /// value. We test for both of them.
    template <typename opt, bool is_multiple>
    bool handle_opt_with_arg(std::string_view opt_str) {
        using opt_type = typename opt::type;

        /// --option=value or short opt.
        if (opt_str.size() > opt::name.len) {
            /// Parse the rest of the option as the value if we have a '=' or if this is a short option.
            if (opt_str[opt::name.len] == '=' or requires { opt::is_short; }) {
                /// Otherwise, parse the value.
                auto opt_start_offs = opt::name.len + (opt_str[opt::name.len] == '=');
                const auto opt_name = opt_str.substr(0, opt_start_offs);
                const auto opt_val = opt_str.substr(opt_start_offs);
                dispatch_option_with_arg<opt, is_multiple>(opt_name, opt_val);
                return true;
            }

            /// Otherwise, this isn’t the right option.
            return false;
        }

        /// Handle the option. If we get here, we know that the option name that we’ve
        /// encountered matches the option name exactly.
        else {
            /// If this is a func option that doesn’t take arguments, just call the callback and we’re done.
            if constexpr (detail::is<opt_type, detail::callback_noarg_type>) {
                opt::callback(user_data, opt_str);
                return true;
            }

            /// Otherwise, try to consume the next argument as the option value.
            else {
                /// No more command line arguments left.
                if (++argi == argc) {
                    handle_error("Missing argument for option \"", opt_str, "\"");
                    return false;
                }

                /// Parse the argument.
                dispatch_option_with_arg<opt, is_multiple>(opt_str, argv[argi]);
                return true;
            }
        }
    }

    /// Handle an option. The parser calls this on each non-positional option.
    template <typename opt>
    bool handle_regular_impl(std::string_view opt_str) {
        /// If the supplied string doesn’t start with the option name, move on to the next option
        if (not opt_str.starts_with(opt::name.sv())) return false;

        /// Check if this option accepts multiple values.
        using base_type = detail::base_type_t<typename opt::type>;
        static constexpr bool is_multiple = requires { opt::is_multiple; };
        if constexpr (not is_multiple and not detail::is_callback<base_type>) {
            /// Duplicate options are not allowed by default.
            if (found<opt::name>()) {
                std::string errmsg;
                errmsg += "Duplicate option: \"";
                errmsg += opt_str;
                errmsg += "\"";
                handle_error(std::move(errmsg));
                return false;
            }
        }

        /// Flags and callbacks that don't have arguments.
        if constexpr (not detail::has_argument<base_type>) {
            /// Check if the name of this flag matches the entire option string that
            /// we encountered. If we’re just a prefix, then we don’t handle this.
            if (opt_str != opt::name.sv()) return false;

            /// Mark the option as found. That’s all we need to do for flags.
            set_found<opt::name>();

            /// If it’s a callable, call it.
            if constexpr (detail::is_callback<base_type>) {
                /// The builtin help option is handled here. We pass the help message as an argument.
                if constexpr (requires { opt::is_help_option; }) invoke_help_callback<opt>();

                /// If it’s not the help option, just invoke it.
                else { opt::callback(user_data, opt_str); }
            }

            /// Option has been handled.
            return true;
        }

        /// Handle an option that may take an argument.
        else { return handle_opt_with_arg<opt, is_multiple>(opt_str); }
    }

#undef INVOKE

    template <typename opt>
    bool handle_positional_impl(std::string_view opt_str) {
        /// If we've already encountered this positional option, then return.
        static constexpr bool is_multiple = requires { opt::is_multiple; };
        if constexpr (not is_multiple) {
            if (found<opt::name>()) return false;
        }

        /// Otherwise, attempt to parse this as the option value.
        set_found<opt::name>();
        if constexpr (is_multiple) ref<opt::name>().push_back(make_arg<typename opt::type>(opt_str));
        else ref<opt::name>() = make_arg<typename opt::type>(opt_str.data());
        return true;
    }

    /// Invoke handle_regular_impl on every option until one returns true.
    bool handle_regular(std::string_view opt_str) {
        const auto handle = [this]<typename opt>(std::string_view str) {
            /// `this->` is to silence a warning.
            if constexpr (detail::is_positional_v<opt>) return false;
            else return this->handle_regular_impl<opt>(str);
        };

        return (handle.template operator()<opts>(opt_str) or ...);
    };

    /// Invoke handle_positional_impl on every option until one returns true.
    bool handle_positional(std::string_view opt_str) {
        const auto handle = [this]<typename opt>(std::string_view str) {
            /// `this->` is to silence a warning.
            if constexpr (detail::is_positional_v<opt>) return this->handle_positional_impl<opt>(str);
            else return false;
        };

        return (handle.template operator()<opts>(opt_str) or ...);
    };

    void parse() {
        /// Main parser loop.
        for (argi = 1; argi < argc; argi++) {
            std::string_view opt_str{argv[argi]};

            /// Attempt to handle the option.
            if (not handle_regular(opt_str) and not handle_positional(opt_str)) {
                std::string errmsg;
                errmsg += "Unrecognized option: \"";
                errmsg += opt_str;
                errmsg += "\"";
                handle_error(std::move(errmsg));
            }

            /// Stop parsing if there was an error.
            if (has_error) return;
        }

        /// Make sure all required options were found.
        Foreach<opts...>([&]<typename opt>() {
            if (not found<opt::name>() and opt::is_required) {
                std::string errmsg;
                errmsg += "Option \"";
                errmsg += opt::name.sv();
                errmsg += "\" is required";
                handle_error(std::move(errmsg));
            }
        });
    }

public:
    /// \brief Parse command line options.
    ///
    /// \param argc The argument count.
    /// \param argv The arguments (including the program name).
    /// \param user_data User data passed to any func\<\> options that accept a \c void*.
    /// \param error_handler A callback that is invoked whenever an error occurs. If
    ///        \c nullptr is passed, the default error handler is used. The error handler
    ///        should return \c true if parsing should continue and \c false otherwise.
    /// \return The parsed option values.
    static auto parse(
        int argc,
        const char* const* const argv,
        std::function<bool(std::string&&)> error_handler = nullptr,
        void* user_data = nullptr
    ) -> optvals_type {
        /// Initialise state.
        This self;
        if (error_handler) self.error_handler = error_handler;
        else self.error_handler = [&](auto&& e) { return self.default_error_handler(std::forward<decltype(e)>(e)); };
        self.argc = argc;
        self.user_data = user_data;

        /// Safe because we don’t attempt to modify argv anyway. This
        /// is just so we can pass in both e.g. a `const char**` and a
        /// `char **`.
        self.argv = const_cast<const char**>(argv);

        /// Parse the options.
        self.parse();
        return std::move(self.optvals);
    }
};

} // namespace detail

/// ===========================================================================
///  API
/// ===========================================================================
/// Main command-line options type.
template <typename... opts>
using clopts = detail::clopts_impl<opts...>;

/// Values for an option.
using detail::values;

/// Base option type.
template <
    detail::static_string _name,
    detail::static_string _description = "",
    typename type = std::string,
    bool required = false>
struct option : detail::opt_impl<_name, _description, type, required> {};

namespace experimental {
/// Base short option type.
template <
    detail::static_string _name,
    detail::static_string _description = "",
    typename _type = std::string,
    bool required = false>
struct short_option : detail::opt_impl<_name, _description, _type, required> {
    static constexpr inline decltype(_name) name = _name;
    static constexpr inline decltype(_description) description = _description;
    static constexpr inline bool is_flag = std::is_same_v<_type, bool>;
    static constexpr inline bool is_required = required;
    static constexpr inline bool is_short = true;
    static constexpr inline bool option_tag = true;

    constexpr short_option() = delete;
};
} // namespace experimental

/// A file.
template <typename contents_type_t = std::string, typename path_type_t = std::filesystem::path>
struct file {
    using contents_type = contents_type_t;
    using path_type = path_type_t;
    using element_type = typename contents_type::value_type;
    using element_pointer = std::add_pointer_t<element_type>;
    static constexpr bool is_file_data = true;

public:
    /// The file path.
    path_type path;

    /// The contents of the file.
    contents_type contents;
};

/// For backwards compatibility.
using file_data = file<>;

/// A positional option.
template <
    detail::static_string _name,
    detail::static_string _description,
    typename _type = std::string,
    bool required = true>
struct positional : option<_name, _description, _type, required> {
    using is_positional_ = std::true_type;
};

/// Func option implementation.
template <
    detail::static_string _name,
    detail::static_string _description,
    typename lambda,
    bool required = false>
struct func_impl : option<_name, _description, typename lambda::type, required> {
    static constexpr inline typename lambda::lambda callback = {};
};

/// A function option.
template <
    detail::static_string _name,
    detail::static_string _description,
    auto cb,
    bool required = false>
struct func : func_impl<_name, _description, detail::make_lambda<cb>, required> {};

/// A flag option.
///
/// Flags are never required because that wouldn’t make much sense.
template <
    detail::static_string _name,
    detail::static_string _description = "">
struct flag : option<_name, _description, bool, false> {};

/// The help option.
template <auto _help_cb = detail::default_help_handler>
struct help : func<"--help", "Print this help information", [] {}> {
    static constexpr decltype(_help_cb) help_callback = _help_cb;
    static constexpr inline bool is_help_option = true;
};

/// Multiple meta-option.
template <typename opt>
struct multiple : option<opt::name, opt::description, std::vector<typename opt::actual_type>, opt::is_required> {
    using base_type = typename opt::type;
    using type = std::vector<typename opt::type>;
    static_assert(not detail::is<base_type, bool>, "Type of multiple<> cannot be bool");
    static_assert(not detail::is<base_type, detail::callback_arg_type>, "Type of multiple<> cannot be a callback");
    static_assert(not detail::is<base_type, detail::callback_noarg_type>, "Type of multiple<> cannot be a callback");

    constexpr multiple() = delete;
    static constexpr inline bool is_multiple = true;
    using is_positional_ = detail::positional_t<opt>;
};

/// Alias declaration.
template <detail::static_string... _names>
struct aliases : option<"_", "_", detail::noop_tag, false> {
    static constexpr std::tuple names = {_names...};
    static constexpr inline bool is_aliases = true;
};

} // namespace command_line_options

#undef CLOPTS_STRLEN
#undef CLOPTS_STRCMP
#undef CLOPTS_ERR
#endif // CLOPTS_H