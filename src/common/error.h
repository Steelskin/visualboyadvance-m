#ifndef VBAM_COMMON_ERROR_H_
#define VBAM_COMMON_ERROR_H_

#include <cassert>
#include <functional>

#include "nonstd/variant.hpp"

enum class VbamError {
    InternalError,
    GenericCharNotHex,
    GbaCheatGenInvalidLength,
    GbaCheatGenNoColon,
    GbaCheatGSAInvalidLength,
    GbaCheatChtNoEqual,
    GbaCheatChtNoComma,
    GbaCheatChtInvalidValue,
};

// A class holding an error or a value. Used to return a value or an explicit error from a function.
template <typename ValueType>
class VbamErrorOr final {
public:
    VbamErrorOr(ValueType&& value) : value_(std::move(value)) {}
    VbamErrorOr(VbamError&& error) : value_(std::move(error)) {}
    ~VbamErrorOr() = default;

    // Returns true if this hold an error, otherwise false.
    bool is_error() const { return nonstd::holds_alternative<VbamError>(value_); }
    // Returns true if the value is present, otherwise false.
    bool is_value() const { return nonstd::holds_alternative<ValueType>(value_); }
    operator bool() const { return is_value(); }

    // Returns the value if held, otherwise asserts.
    ValueType&& value() {
        assert(is_value());
        return std::move(nonstd::get<ValueType>(value_));
    }
    // Returns the value if held, otherwise returns `fallback`.
    ValueType&& value(ValueType&& fallback) {
        if (is_value()) {
            return std::move(nonstd::get<ValueType>(value_));
        } else {
            return std::forward(fallback);
        }
    }
    // Returns the value if held, otherwise returns a lazily evaluated fallback.
    // `fallback_provider` is not called if the value is held.
    ValueType&& value(std::function<ValueType && ()> fallback_provider) {
        if (is_value()) {
            return std::move(nonstd::get<ValueType>(value_));
        } else {
            return fallback_provider();
        }
    }

    // Returns the error if held, otherwise asserts.
    VbamError error() const {
        assert(is_error());
        return nonstd::get<VbamError>(value_);
    }

private:
    nonstd::variant<ValueType, VbamError> value_;
};

#endif  // VBAM_COMMON_ERROR_H_
