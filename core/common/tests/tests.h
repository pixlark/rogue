#pragma once

#include <test-platform.h>

#include "../stack_vector.h"
class StackVectorTests : public ITestSystem {
public:
    void can_construct(const Test& test) {
        stack_vector<int, 10> empty_vec;
        assert_equal(empty_vec.get_count(), 0);

        stack_vector<int, 10> three_item_vec{1, 2, 3};
        assert_equal(three_item_vec.get_count(), 3);
    }

    void throws_on_out_of_bounds(const Test& test) {
        stack_vector<int, 5> full_vec{1, 2, 3, 4, 5};
        assert_equal(full_vec.get_count(), 5);

        assert_throws(std::runtime_error, full_vec.push_back(6));
    }

    void can_use_range_based_for_loop(const Test& test) {
        stack_vector<int, 10> vec{1, 2, 3, 4, 5};

        std::vector<int> traversed;
        for (auto& i : vec) {
            traversed.push_back(i);
        }

        assert_equal(traversed, (std::vector<int>{1, 2, 3, 4, 5}));
    }

    void setup(System& system) {
        add_test(can_construct);
        add_test(throws_on_out_of_bounds);
        add_test(can_use_range_based_for_loop);
    }
};
