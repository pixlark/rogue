#include <stdexcept>
#include <vector>
#include <optional>

#include <cgreen/cgreen.h>
using namespace cgreen;

#include "util.h"

//
// stack_vector
//

Describe(stack_vector_SUT);
BeforeEach(stack_vector_SUT) {}
AfterEach(stack_vector_SUT) {}

Ensure(stack_vector_SUT, can_construct) {
    stack_vector<int, 10> empty_vec;
    assert_that(empty_vec.get_count(), is_equal_to(0));

    stack_vector<int, 10> three_item_vec{1, 2, 3};
    assert_that(three_item_vec.get_count(), is_equal_to(3));
}

Ensure(stack_vector_SUT, throws_on_out_of_bounds) {
    stack_vector<int, 5> full_vec{1, 2, 3, 4, 5};
    assert_that(full_vec.get_count(), is_equal_to(5));

    assert_throws(std::runtime_error, full_vec.push_back(6));
}

Ensure(stack_vector_SUT, can_use_range_based_for_loop) {
    stack_vector<int, 10> vec{1, 2, 3, 4, 5};

    std::vector<int> traversed;
    for (auto& i : vec) {
        traversed.push_back(i);
    }

    assert_that((traversed == std::vector<int>{1, 2, 3, 4, 5}));
}

int main() {
    TestSuite* suite = cgreen::create_test_suite();
    add_test_with_context(suite, stack_vector_SUT, can_construct);
    add_test_with_context(suite, stack_vector_SUT, throws_on_out_of_bounds);
    add_test_with_context(suite, stack_vector_SUT, can_use_range_based_for_loop);
    return run_test_suite(suite, create_text_reporter());
}
