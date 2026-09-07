#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include "crykit.hpp"

namespace utils{
    ConstBuffer do_padding(ConstBuffer input, size_t block_size);
    MutBuffer undo_padding(ConstBuffer input);
}

struct GF28 {
    uint8_t v;
    static uint8_t gf_mul(uint8_t a, uint8_t b);
    public:
        GF28(uint8_t v=0 ) : v(v) {}
        uint8_t value() const {return v;}

        GF28 operator+(GF28 o) const {return v ^ o.v;}
        GF28 operator-(GF28 o) const {return v ^ o.v;}
        GF28 operator+=(GF28 o) { v ^= o.v; return *this; }
        GF28 operator-=(GF28 o) { v ^= o.v; return *this; }

        GF28 operator*(GF28 o) const {return GF28(gf_mul(v, o.v));}
        GF28 operator*=(GF28 o) {v = gf_mul(v, o.v); return *this;;}

        GF28 inv();
        GF28 operator/(GF28 o) const {return *this * o.inv();}
        bool operator==(GF28 o) const {return v==o.v;}
        bool operator!=(GF28 o) const {return v!=o.v;}

};
template<typename T>
struct Matrix {
    size_t rows_, cols_;
    std::vector<T> data_;
    public:
        Matrix(){}
        Matrix(size_t rows, size_t cols, T fill = T())
            : rows_(rows), cols_(cols), data_(rows * cols, fill) {}

        T& at(size_t i, size_t j){return data_[i * cols_ + j]; }
        const T& at(size_t i, size_t j) const {return data_[i * cols_ + j];}

        size_t rows() const { return rows_;}
        size_t cols() const { return cols_;}

        Matrix operator*(const Matrix& o) const {
            if (cols_ != o.rows_) throw std::invalid_argument("Matrix: size mismatch");
            Matrix result(rows_, o.cols_);
            for (size_t i = 0; i < rows_; ++i)
                for (size_t k = 0; k < cols_; ++k)
                    for (size_t j = 0; j < o.cols_; ++j)
                        result.at(i, j) += at(i, k) * o.at(k, j);
            return result;
        }

        Matrix inv() const {
            if (rows_ != cols_) throw std::invalid_argument("Matrix: not square");
            size_t n = rows_;
            Matrix aug(n, 2 * n);

            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < n; ++j)
                    aug.at(i, j) = at(i, j);
                aug.at(i, n + i) = T(1);
            }

            for (size_t col = 0; col < n; ++col) {
                size_t pivot = n;
                for (size_t row = col; row < n; ++row) {
                    if (aug.at(row, col) != T(0)) { pivot = row; break; }
                }
                if (pivot == n) throw std::runtime_error("Matrix: singular");

                if (pivot != col)
                    for (size_t j = 0; j < 2 * n; ++j)
                        std::swap(aug.at(col, j), aug.at(pivot, j));

                T inv_pivot = aug.at(col, col).inv();
                for (size_t j = 0; j < 2 * n; ++j)
                    aug.at(col, j) *= inv_pivot;

                for (size_t row = 0; row < n; ++row) {
                    if (row == col) continue;
                    T factor = aug.at(row, col);
                    for (size_t j = 0; j < 2 * n; ++j)
                        aug.at(row, j) -= factor * aug.at(col, j);
                }
            }

            Matrix result(n, n);
            for (size_t i = 0; i < n; ++i)
                for (size_t j = 0; j < n; ++j)
                    result.at(i, j) = aug.at(i, n + j);
            return result;
        }
};
