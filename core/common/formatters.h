#pragma once

#include <format>

template<typename T>
    requires std::formattable<T, char>
struct std::formatter<std::optional<T>> : std::formatter<std::string> {
    template <class FormatContext>
    auto format(std::optional<T> opt, FormatContext& context) const {
        if (opt.has_value()) {
            return formatter<string>::format(
                std::format("std::optional({})", opt.value()),
                context
            );
        } else {
            return formatter<string>::format(
                "std::nullopt", context
            );
        }
    }
};
