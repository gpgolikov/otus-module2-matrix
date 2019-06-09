#pragma once

#include <tuple>
#include <utility>
#include <map>

namespace griha {

namespace details {

template <typename T, size_t... I>
auto make_mono_tuple_helper(std::index_sequence<I...>) {
    T a[sizeof...(I)] = {};
    return std::tuple<std::remove_reference_t<decltype(a[I])>...>();
}

template <typename T, size_t N>
using make_mono_tuple = 
    decltype(make_mono_tuple_helper<T>(std::make_index_sequence<N>()));

template <typename Tuple, typename... Ts>
using tuple_extend_t = decltype(std::tuple_cat(Tuple{}, std::declval<std::tuple<Ts...>>()));

} // namespace details

template <typename T, T Def, size_t Dim = 2>
class Matrix {

    static_assert(Dim >= 2, "Dimention of matrix should be 2 or greater");

public:
    using size_type = size_t;
    using index_type = details::make_mono_tuple<const size_type, Dim>;
    using value_type = details::tuple_extend_t<index_type, T>;

private:
    using buffer_type = std::map<index_type, T>;

    template <size_type Level, T DefV>
    class matrix_accessor {
    public:
        using middle_index_type =
            details::make_mono_tuple<const size_type, Dim - Level + 1>;

        matrix_accessor(Matrix& m, middle_index_type idx) 
            : matrix_(m)
            , index_(idx) {}

        matrix_accessor(const matrix_accessor&) = delete;
        matrix_accessor& operator= (const matrix_accessor&) = delete;
        matrix_accessor(matrix_accessor&&) = delete;
        matrix_accessor& operator= (matrix_accessor&&) = delete;

        auto operator[] (size_type idx) && {
            return matrix_accessor<Level - 1, DefV>{
                matrix_,
                std::tuple_cat(index_, std::make_tuple(idx))
            };
        }

    private:
        Matrix& matrix_;
        middle_index_type index_;
    };

    template <T DefV>
    class matrix_accessor<1, DefV> {
    public:
        matrix_accessor(Matrix& m, index_type idx)
            : matrix_(m)
            , index_(idx) {
            
            auto it = matrix_.buffer_.find(index_);
            if (it != matrix_.buffer_.end())
                value_ = it->second;
        }

        operator T() const {
            return value_;
        }

        matrix_accessor& operator= (T v) & {
            value_ = v;
            return *this;
        }

        matrix_accessor& operator= (const matrix_accessor& acc) & {
            return *this = acc.value_;
        }

        matrix_accessor&& operator= (T v) && {
            if (v == DefV) {
                if (value_ != DefV)
                    matrix_.buffer_.erase(index_);
            } else {
                matrix_.buffer_.insert_or_assign(index_, v);
            }
            value_ = v;
            return std::move(*this);
        }

        matrix_accessor&& operator= (const matrix_accessor& acc) && {
            return std::move(*this) = acc.value_;
        }

        friend bool operator== (const matrix_accessor& lhs, const matrix_accessor& rhs) {
            return lhs.value_ == rhs.value_;
        }

    private:
        Matrix& matrix_;
        index_type index_;
        T value_ = { DefV };
    };

public:
    class const_iterator;
    class iterator {
        friend class const_iterator;

    public:
        using reference = details::tuple_extend_t<index_type, T&>;

        explicit iterator(typename buffer_type::iterator i) : it_(std::move(i)) {}

    public:
        iterator() = default;

        iterator(const iterator&) = default;
        iterator(iterator&&) = default;

        iterator& operator= (const iterator&) = default;
        iterator& operator= (iterator&&) = default;

        iterator& operator++ () {
            ++it_; return *this;
        }

        iterator operator++ (int) {
            return iterator(it_++);
        }

        iterator& operator-- () {
            --it_; return *this;
        }

        iterator operator-- (int) {
            return iterator(it_--);
        }

        reference operator* () {
            return std::tuple_cat(it_->first, std::tie(it_->second));
        }

        friend bool operator!= (const iterator& lhs, const iterator& rhs) {
            return lhs.it_ != rhs.it_;
        }

    private:
        typename buffer_type::iterator it_;
    };

    class const_iterator {
    public:
        using reference = details::tuple_extend_t<index_type, T>;

        explicit const_iterator(typename buffer_type::const_iterator i) : it_(std::move(i)) {}

    public:
        const_iterator() = default;

        const_iterator(const iterator& src) : it_(src.it_) {}

        const_iterator(const const_iterator&) = default;
        const_iterator(const_iterator&&) = default;

        const_iterator& operator= (const const_iterator&) = default;
        const_iterator& operator= (const_iterator&&) = default;

        const_iterator& operator++ () {
            ++it_; return *this;
        }

        const_iterator operator++ (int) {
            return const_iterator(it_++);
        }

        const_iterator& operator-- () {
            --it_; return *this;
        }

        const_iterator operator-- (int) {
            return const_iterator(it_--);
        }

        reference operator* () {
            return std::tuple_cat(it_->first, std::tie(it_->second));
        }

        friend bool operator!= (const const_iterator& lhs, const const_iterator& rhs) {
            return lhs.it_ != rhs.it_;
        }

    private:
        typename buffer_type::const_iterator it_;
    };

public:

    iterator begin() { return iterator(buffer_.begin()); }
    const_iterator begin() const { return const_iterator(buffer_.begin()); }
    const_iterator cbegin() const { return const_iterator(buffer_.cbegin()); }

    iterator end() { return iterator(buffer_.end()); }
    const_iterator end() const { return const_iterator(buffer_.end()); }
    const_iterator cend() const { return const_iterator(buffer_.cend()); }

    auto operator[] (size_type idx) {
        return matrix_accessor<Dim, Def>{ *this, std::make_tuple(idx) };
    }

    size_type size() const { return buffer_.size(); }

private:
    buffer_type buffer_;
};

} // namespace griha