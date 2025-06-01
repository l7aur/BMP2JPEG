#include "Mat8x8.h"

namespace Util {
    Mat8x8::Mat8x8(const std::array<std::array<double, 8>, 8> &mat)
        : data(mat)
    {
    }

    Mat8x8::Mat8x8() {
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                data[i][j] = 0.0;
    }

    double Mat8x8::at(const int row, const int col) const {
        return data[row][col];
    }

    Mat8x8 Mat8x8::transpose() const {
        Mat8x8 result;
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                result.data[i][j] = data[j][i];
        return result;
    }

    Mat8x8 Mat8x8::operator*(const Mat8x8 &other) const {
        Mat8x8 result{};
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                for (int k = 0; k < 8; k++)
                    result.data[i][j] += data[i][k] * other.data[k][j];
        return result;
    }

    std::ostream & operator<<(std::ostream &os, const Mat8x8 &mat) {
        for (int i = 0; i < 8; i++, std::cout << std::endl)
            for (int j = 0; j < 8; j++)
                os << mat.data[i][j] << " ";
        return os;
    }
}
