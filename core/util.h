#pragma once

#include <array>
#include <stdexcept>
#include <initializer_list>
#include <optional>

template <typename T, std::size_t N>
class stack_vector {
    std::array<std::optional<T>, N> arr;

    class pseudo_iterator {
        friend stack_vector<T, N>;

        stack_vector<T, N>& vec;
        int index;

        pseudo_iterator(stack_vector<T, N>& vec)
            : vec(vec), index(0) {}

        pseudo_iterator(stack_vector<T, N>& vec, int index)
            : vec(vec), index(index) {}

    public:
        T& operator*() {
            return this->vec[this->index];
        }

        void operator++() {
            this->index++;
        }

        friend bool operator!=(const pseudo_iterator& a, const pseudo_iterator b) {
            return a.index != b.index;
        }
    };

    int count;

public:
    stack_vector()
        : count(0) {
        this->arr.fill(std::nullopt);
    }
    explicit stack_vector(std::initializer_list<T> list)
        : count(0) {
        this->arr.fill(std::nullopt);
        for (auto it = list.begin(); it < list.end(); it++) {
            this->push_back(*it);
        }
    }

    void push_back(T element) {
        if (this->count >= N) {
            throw std::runtime_error("Pushed too many elements to a stack_vector!");
        }

        this->arr[this->count++] = element;
    }

    T& operator[](int index) {
        if (index < 0 || index >= this->count) {
            throw std::runtime_error("Indexed stack_vector out of bounds!");
        }

        return this->arr[index].value();
    }

    int get_count() {
        return this->count;
    }

    auto begin() {
        return stack_vector<T, N>::pseudo_iterator(*this);
    }

    auto end() {
        return stack_vector<T, N>::pseudo_iterator(*this, this->count);
    }

    friend auto begin(stack_vector<T, N>& v) {
        return v.begin();
    }

    friend auto end(stack_vector<T, N>& v) {
        return v.end();
    }
};
