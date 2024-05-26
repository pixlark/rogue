#pragma once

#include <any>
#include <concepts>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class ITestSystem;

// Meta-structure for preparing and running unit tests. This needs only
// ever be instantiated once.
class TestPlatform {
    std::vector<std::pair<std::string, std::function<std::unique_ptr<ITestSystem>()>>> systems;
    std::size_t longest_test_name = 0;

public:
    TestPlatform();

    template <typename T>
        requires std::derived_from<T, ITestSystem>
    void add_system(std::string name) {
        this->systems.emplace_back(
            name,
            []() { return std::make_unique<T>(); }
        );
    }

    void notify_test_name_length(std::size_t length);
    int run();
};

// Do not use -- internal class used for handling assertion failures
class BooleanAssertionException : public std::runtime_error {
public:
    const char* filename;
    int line_number;
    const char* a_var_string;

    template <typename T>
    BooleanAssertionException(
        T a,
        const char* filename, int line_number,
        const char* a_var_string
    ) : std::runtime_error("assertion failure (if you can see this exception, something internal to test-platform has gone wrong!)"),
        filename(filename), line_number(line_number),
        a_var_string(a_var_string) {}
};

// Do not use -- internal class used for handling assertion failures
class EqualityAssertionException : public std::runtime_error {
public:
    const char* assert_name;
    const char* inequality;
    const char* filename;
    int line_number;
    const char* a_var_string;
    const char* b_var_string;
    std::string a_value_string;
    std::string b_value_string;

    template <typename T>
    EqualityAssertionException(
        T a, T b,
        const char* assert_name,
        const char* inequality,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string,
        std::string a_value_string, std::string b_value_string
    ) : std::runtime_error("assertion failure (if you can see this exception, something internal to test-platform has gone wrong!)"),
        assert_name(assert_name),
        inequality(inequality),
        filename(filename), line_number(line_number),
        a_var_string(a_var_string), b_var_string(b_var_string),
        a_value_string(a_value_string), b_value_string(b_value_string) {}
};

// Do not use -- internal class used for handling assertion failures
class ThrowsAssertionException : public std::runtime_error {
public:
    const char* filename;
    int line_number;
    const char* exception_string;
    const char* expression_string;

    ThrowsAssertionException(
        const char* filename, int line_number,
        const char* exception_string,
        const char* expression_string
    ) : std::runtime_error("assertion failure (if you can see this exception, something internal to test-platform has gone wrong!)"),
        filename(filename), line_number(line_number),
        exception_string(exception_string),
        expression_string(expression_string) {}
};

template <typename T>
concept not_formattable = requires(T t)
{
    !std::formattable<T, char>;
};

template<typename T>
    requires std::floating_point<T>
bool approx_equal(T a, T b) {
    if (a == b) {
        return true;
    }

    if (std::isinf(a) || std::isinf(b)) {
        return false;
    }

    T abs_diff = std::abs(a - b);

    T epsilon = std::numeric_limits<T>::epsilon();
    return abs_diff <= epsilon;
}

// Helper class passed to all unit tests.
class Test {
    friend TestPlatform;

    const TestPlatform& platform;
    const ITestSystem& system;

    Test(const TestPlatform& platform, const ITestSystem& system);

public:
    void _assert_true(
        bool a,
        const char* filename, int line_number,
        const char* a_var_string
    ) const {
        if (!a) {
            throw BooleanAssertionException(a, filename, line_number, a_var_string);
        }
    }

    template <typename T>
        requires std::equality_comparable<T> && std::formattable<T, char>
    void _assert_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a != b) {
            throw EqualityAssertionException(a, b, "assert_equal", "!=", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::equality_comparable<T>
    void _assert_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a != b) {
            throw EqualityAssertionException(a, b, "assert_equal", "!=", filename, line_number, a_var_string, b_var_string, "<not formattable>", "<not formattable>");
        }
    }

    template <typename T>
        requires std::floating_point<T>
    void _assert_approx_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (!approx_equal<T>(a, b)) {
            throw EqualityAssertionException(a, b, "assert_approx_equal", "!=", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::equality_comparable<T> && std::formattable<T, char>
    void _assert_not_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a == b) {
            throw EqualityAssertionException(a, b, "assert_not_equal", "==", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::equality_comparable<T>
    void _assert_not_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a == b) {
            throw EqualityAssertionException(a, b, "assert_not_equal", "==", filename, line_number, a_var_string, b_var_string, "<not formattable>", "<not formattable>");
        }
    }

    template <typename T>
        requires std::floating_point<T>
    void _assert_approx_not_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (approx_equal<T>(a, b)) {
            throw EqualityAssertionException(a, b, "assert_approx_not_equal", "==", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::three_way_comparable<T> && std::formattable<T, char>
    void _assert_less_than(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a >= b) {
            throw EqualityAssertionException(a, b, "assert_less_than", ">=", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::three_way_comparable<T> && std::formattable<T, char>
    void _assert_less_than_or_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a > b) {
            throw EqualityAssertionException(a, b, "assert_less_than_or_equal", ">", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::three_way_comparable<T> && std::formattable<T, char>
    void _assert_greater_than(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a <= b) {
            throw EqualityAssertionException(a, b, "assert_greater_than", "<=", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename T>
        requires std::three_way_comparable<T> && std::formattable<T, char>
    void _assert_greater_than_or_equal(
        T a, T b,
        const char* filename, int line_number,
        const char* a_var_string, const char* b_var_string
    ) const {
        if (a < b) {
            throw EqualityAssertionException(a, b, "assert_greater_than_or_equal", "<", filename, line_number, a_var_string, b_var_string, std::format("{}", a), std::format("{}", b));
        }
    }

    template <typename E>
        requires std::derived_from<E, std::exception>
    void _assert_throws(
        std::function<void()> func,
        const char* filename, int line_number,
        const char* exception_string,
        const char* expression_string
    ) const {
        try {
            func();
        } catch (E) {
            return;
        }

        throw ThrowsAssertionException(filename, line_number, exception_string, expression_string);
    }
};

#define assert_true(a) do { \
        auto _a = (a); \
        test._assert_true(_a, __FILE__, __LINE__, #a); \
    } while (0)

#define assert_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_approx_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_approx_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_not_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_not_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_approx_not_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_approx_not_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_less_than(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_less_than(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_less_than_or_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_less_than_or_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_greater_than(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_greater_than(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_greater_than_or_equal(a, b) do { \
        auto _a = (a); \
        auto _b = (b); \
        test._assert_greater_than_or_equal(_a, _b, __FILE__, __LINE__, #a, #b); \
    } while (0)

#define assert_throws(exception, expression) test._assert_throws<exception>([&]() { (expression); }, __FILE__, __LINE__, #exception, #expression)

// Helper class passed to test systems.
class System {
    friend TestPlatform;

    TestPlatform& platform;

    typedef std::function<void(ITestSystem*, const Test&)> test_t;
    std::vector<std::pair<std::string, test_t>> tests;

    System(TestPlatform& platform);

public:
    void _add_test(std::string name, test_t func);
};

#define add_test(test_func) system._add_test(\
    #test_func, \
    [&](ITestSystem* t, const Test& test) { \
        auto f = &std::remove_pointer<decltype(this)>::type::test_func; \
        (static_cast<decltype(this)>(t)->*f)(test); \
    })

// Interface describing a SUT (system under test) which contains unit tests.
// A paremeterless constructor and destructor can perform setup/teardown tasks.
// A separate instance of the system will be created for every unit test, as well
// as once for the setup function.
class ITestSystem {
public:
    virtual ~ITestSystem() = 0 {}
    virtual void setup(System& system) = 0;
};
