#include "test-platform.h"

#include <print>

//
// TestPlatform
//

#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_BRED    "\033[91m"
#define ANSI_BGREEN  "\033[92m"
#define ANSI_BYELLOW "\033[93m"
#define ANSI_BBLUE   "\033[94m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_RESET   "\033[0m"
#define RED(s)       ANSI_RED     s ANSI_RESET
#define GREEN(s)     ANSI_GREEN   s ANSI_RESET
#define YELLOW(s)    ANSI_YELLOW  s ANSI_RESET
#define BBLUE(s)     ANSI_BBLUE   s ANSI_RESET
#define BRED(s)      ANSI_BRED    s ANSI_RESET
#define BGREEN(s)    ANSI_BGREEN  s ANSI_RESET
#define BYELLOW(s)   ANSI_BYELLOW s ANSI_RESET
#define BLUE(s)      ANSI_BLUE    s ANSI_RESET
#define BOLD(s)      ANSI_BOLD    s ANSI_RESET

TestPlatform::TestPlatform() {}

void TestPlatform::notify_test_name_length(std::size_t length) {
    this->longest_test_name = std::max(this->longest_test_name, length);
}

int TestPlatform::run() {
    for (const auto& system_pair : this->systems) {
        const std::string_view& system_name(system_pair.first);
        const auto& system_factory = system_pair.second;

        std::println(BOLD("[{}]"), system_name);

        this->longest_test_name = 0;
        auto tests = [&]() {
            std::unique_ptr<ITestSystem> setup_system = system_factory();
            System setup_handle(*this);
            setup_system->setup(setup_handle);
            return setup_handle.tests;
        }();

        for (const auto& test_pair : tests) {
            const std::string_view& test_name(test_pair.first);
            auto test_function(test_pair.second);

            std::string test_name_padding(std::max(0ull, this->longest_test_name - test_name.length()), ' ');

            std::print(" {}...{} ", test_name, test_name_padding);
            std::fflush(stdout);

            std::unique_ptr<ITestSystem> system = system_factory();
            Test test(*this, *system);

            try {
                test_function(&*system, test);
            } catch (BooleanAssertionException e) {
                std::println(
                    RED("NO") "\n\n"
                    RED("Assertion failed!\n")
                    BBLUE("at {}:{}\n")
                    " assert_true({})\n"
                    BBLUE(" because\n")
                    "   {} is false\n",
                    // BBLUE(" where\n")
                    // "   {} = {}",
                    e.filename, e.line_number,
                    e.a_var_string,
                    e.a_var_string
                    // e.a_var_string, e.a_value_string
                );
                continue;
            } catch (EqualityAssertionException e) {
                int a_var_len = std::strlen(e.a_var_string);
                int b_var_len = std::strlen(e.b_var_string);
                int var_width = std::max(a_var_len, b_var_len);
                std::string a_var_padding(std::max(0, var_width - a_var_len), ' ');
                std::string b_var_padding(std::max(0, var_width - b_var_len), ' ');
                std::println(
                    RED("NO") "\n\n"
                    RED("Assertion failed!\n")
                    BBLUE("at {}:{}\n")
                    " {}({}, {})\n"
                    BBLUE(" because\n")
                    "   {} {} {}\n"
                    BBLUE(" where\n")
                    "   {}{} = {}\n"
                    "   {}{} = {}\n",
                    e.filename, e.line_number,
                    e.assert_name, e.a_var_string, e.b_var_string,
                    e.a_var_string, e.inequality, e.b_var_string,
                    e.a_var_string, a_var_padding, e.a_value_string,
                    e.b_var_string, b_var_padding, e.b_value_string
                );
                continue;
            } catch (ThrowsAssertionException e) {
                std::println(
                    RED("NO") "\n\n"
                    RED("Assertion failed!\n")
                    BBLUE("at {}:{}\n")
                    " assert_throws({}, {})\n"
                    BBLUE(" because\n")
                    "   exception {} was not thrown\n",
                    e.filename, e.line_number,
                    e.exception_string, e.expression_string,
                    e.exception_string
                );
            } catch (const std::exception& e) {
                std::println(
                    RED("NO") "\n\n"
                    RED("Assertion failed!\n")
                    BBLUE(" because\n")
                    "   an exception was thrown\n"
                    BBLUE(" where\n")
                    "  exception.what() = {}\n",
                    e.what()
                );
            }

            std::println(GREEN("OK"));
        }
    }

    return 0;
}

//
// Test
//

Test::Test(const TestPlatform& platform, const ITestSystem& system)
    : platform(platform), system(system) {}

// specialize assert_equal and assert_not_equal for floats
// and doubles to avoid floating point equality issues

// template<>
// void Test::_assert_equal<float>(
//     float a, float b,
//     const char* filename, int line_number,
//     const char* a_var_string, const char* b_var_string
// ) const {
//     // TODO
// }

// template<>
// void Test::_assert_equal<double>(
//     double a, double b,
//     const char* filename, int line_number,
//     const char* a_var_string, const char* b_var_string
// ) const {
//     // TODO
// }

// template<>
// void Test::_assert_not_equal<float>(
//     float a, float b,
//     const char* filename, int line_number,
//     const char* a_var_string, const char* b_var_string
// ) const {
//     // TODO
// }

// template<>
// void Test::_assert_not_equal<double>(
//     double a, double b,
//     const char* filename, int line_number,
//     const char* a_var_string, const char* b_var_string
// ) const {
//     // TODO
// }

//
// System
//

System::System(TestPlatform& platform)
    : platform(platform) {}

void System::_add_test(std::string name, System::test_t func) {
    this->platform.notify_test_name_length(name.length());
    this->tests.emplace_back(name, func);
}
