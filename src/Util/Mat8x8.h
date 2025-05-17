#pragma once
#include <array>
#include <cmath>

namespace Util {

    class Mat8x8 {
    public:
        explicit Mat8x8(const std::array<std::array<double, 8>, 8>& mat);
        explicit Mat8x8();
        constexpr Mat8x8(const std::initializer_list<std::array<double, 8>>& mat);

        [[nodiscard]] static constexpr Mat8x8 dctMat();

        [[nodiscard]] double at(int row, int col) const;
        [[nodiscard]] Mat8x8 transpose() const;
        Mat8x8 operator*(const Mat8x8 &other) const;

    private:
        std::array<std::array<double, 8>, 8> data{};
    };

    constexpr Mat8x8::Mat8x8(const std::initializer_list<std::array<double, 8>> &mat) {
        for (int i = 0; i < 8; i++)
            data[i] = mat.begin()[i];
    }

    constexpr Mat8x8 Mat8x8::dctMat() {
        std::array<double, 8> row1{}; row1.fill(1.0/std::sqrt(8.0));
        std::array<double, 8> row2{}; row2.fill(0.5);
        std::array<double, 8> row3{}; row3.fill(0.5);
        std::array<double, 8> row4{}; row4.fill(0.5);
        std::array<double, 8> row5{}; row5.fill(0.5);
        std::array<double, 8> row6{}; row6.fill(0.5);
        std::array<double, 8> row7{}; row7.fill(0.5);
        std::array<double, 8> row8{}; row8.fill(0.5);
        for (int i = 0; i < 8; i++)
            row2[i] *= std::cos((2 * i + 1) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row3[i] *= std::cos((4 * i + 2) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row4[i] *= std::cos((6 * i + 3) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row5[i] *= std::cos((8 * i + 4) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row6[i] *= std::cos((10 * i + 5) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row7[i] *= std::cos((12 * i + 6) * M_PI / 16.0);
        for (int i = 0; i < 8; i++)
            row8[i] *= std::cos((14 * i + 7) * M_PI / 16.0);
        return Mat8x8({row1, row2, row3, row4, row5, row6, row7, row8});
    }
}

