#include <stdexcept>
#include <vector>
#include <optional>
#include <print>

#include <test-platform.h>

#include <common/formatters.h>
#include <common/tests/tests.h>

class ApproxEqualSystem : public ITestSystem {
public:
    void float_comparisons(const Test& test) {
        assert_approx_equal(100.0, 100.0);
        assert_approx_equal(100.0f, 100.0f);

        assert_approx_not_equal(
            1e10f,
            1.0000001e10f
        );
    }

    void setup(System& system) {
        add_test(float_comparisons);
    }
};

int main() {
    TestPlatform test_platform;
    test_platform.add_system<ApproxEqualSystem>("approx_equal");
    test_platform.add_system<StackVectorTests>("stack_vector");
    return test_platform.run();
}

// #include <cgreen/cgreen.h>
// using namespace cgreen;

// //
// // stack_vector
// //

// Describe(stack_vector_SUT);
// BeforeEach(stack_vector_SUT) {}
// AfterEach(stack_vector_SUT) {}

// Ensure(stack_vector_SUT, can_construct) {
//     stack_vector<int, 10> empty_vec;
//     assert_that(empty_vec.get_count(), is_equal_to(0));

//     stack_vector<int, 10> three_item_vec{1, 2, 3};
//     assert_that(three_item_vec.get_count(), is_equal_to(3));
// }

// Ensure(stack_vector_SUT, throws_on_out_of_bounds) {
//     stack_vector<int, 5> full_vec{1, 2, 3, 4, 5};
//     assert_that(full_vec.get_count(), is_equal_to(5));

//     assert_throws(std::runtime_error, full_vec.push_back(6));
// }

// Ensure(stack_vector_SUT, can_use_range_based_for_loop) {
//     stack_vector<int, 10> vec{1, 2, 3, 4, 5};

//     std::vector<int> traversed;
//     for (auto& i : vec) {
//         traversed.push_back(i);
//     }

//     assert_that((traversed == std::vector<int>{1, 2, 3, 4, 5}));
// }

// int main() {
//     TestSuite* suite = cgreen::create_test_suite();
//     add_test_with_context(suite, stack_vector_SUT, can_construct);
//     add_test_with_context(suite, stack_vector_SUT, throws_on_out_of_bounds);
//     add_test_with_context(suite, stack_vector_SUT, can_use_range_based_for_loop);
//     return run_test_suite(suite, create_text_reporter());
// }
