#include <iostream>
#include <vector>
#include <immintrin.h>
#include "../lodepng.h"
#include <chrono>
#include <map>

using namespace std;

unsigned char addition(unsigned char a, unsigned char b) {
    return min(a + b, 255);
}

unsigned char subtraction(unsigned char a, unsigned char b) {
    return max(a - b, 0);
}

unsigned char grainMerge(unsigned char a, unsigned char b) {
    return min(a - b + 128, 255);
}

unsigned char grainExtract(unsigned char a, unsigned char b) {
    return max(a + b - 128, 0);
}

void readImage(string filename, vector<unsigned char> &image, unsigned &size) {
    lodepng::decode(image, size, size, filename);
}

void saveImage(string filename, vector<unsigned char> &image, int size) {
    lodepng::encode(filename, image, size, size);
}

vector<unsigned char>
plain(vector<unsigned char> &img1, vector<unsigned char> &img2, unsigned char(*operation)(unsigned char, unsigned char)) {

    vector<unsigned char> output(img2.size(), 0);
    for (int i = 0; i < img2.size(); i += 4) {
        output[i] = operation(img1[i], img2[i]);
        output[i + 1] = operation(img1[i + 1], img2[i + 1]);
        output[i + 2] = operation(img1[i + 2], img2[i + 2]);
        output[i + 3] = 255;
    }
    return output;
}

vector<unsigned char> openMP(vector<unsigned char> &img1, vector<unsigned char> &img2,
                             unsigned char(*operation)(unsigned char, unsigned char)) {

    vector<unsigned char> output(img2.size(), 0);
#pragma omp parallel for
    for (int i = 0; i < img2.size(); i += 4) {
        output[i] = operation(img1[i], img2[i]);
        output[i + 1] = operation(img1[i + 1], img2[i + 1]);
        output[i + 2] = operation(img1[i + 2], img2[i + 2]);
        output[i + 3] = 255;
    }

    return output;
}

vector<unsigned char> vectorize(vector<unsigned char> &img1, vector<unsigned char> &img2) {

    vector<unsigned char> output(img2.size(), 0);
    for (int i = 0; i < img1.size(); i += 32) {
        __m256i x = _mm256_load_si256((__m256i *) &img1[i]);
        __m256i y = _mm256_load_si256((__m256i *) &img2[i]);
        __m256i z = _mm256_adds_epu8(x, y);
        _mm256_store_si256((__m256i *) &output[i], z);
    }

    return output;
}

vector<unsigned char> vectorizeOMP(vector<unsigned char> &img1, vector<unsigned char> &img2) {

    vector<unsigned char> output(img2.size(), 0);
#pragma omp parallel for
    for (int i = 0; i < img1.size(); i += 32) {
        __m256i x = _mm256_load_si256((__m256i *) &img1[i]);
        __m256i y = _mm256_load_si256((__m256i *) &img2[i]);
        __m256i z = _mm256_adds_epu8(x, y);
        _mm256_store_si256((__m256i *) &output[i], z);
    }

    return output;
}

vector<string> imageSizes = {"300", "400", "500", "600", "950", "2400"};

map<char, unsigned char (*)(unsigned char, unsigned char)> operations = {
        {'1', addition},
        {'2', subtraction},
        {'3', grainMerge},
        {'4', grainExtract},
};

int main() {
    unsigned size;

    vector<unsigned char> image;

    // Initialize start, difference and end here to not reinitialize in every loop
    auto start = chrono::system_clock::now();
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;

    float averageTime = 0;
    int timesToRepeat = 100;

    char operationNumber;
    cout << "Available operation: \n"
            "1) addition\n"
            "2) subtraction\n"
            "3) grainMerge\n"
            "4) grainExtract\n";
    cout << "Write the number of operation and press Enter: ";
    while (cin >> operationNumber) {
        for (auto imageSize: imageSizes) {
            cout << "\nIMAGE SIZE: " << imageSize << "x" << imageSize << "\n";
            vector<unsigned char> img1, img2;
            readImage("img/" + imageSize + "x" + imageSize + " 1.png", img1, size);
            readImage("img/" + imageSize + "x" + imageSize + " 2.png", img2, size);

            // Plain operation
            for (int i = 0; i < timesToRepeat; i++) {
                start = chrono::system_clock::now();
                image = plain(img1, img2, operations[operationNumber]);
                end = chrono::system_clock::now();
                diff = end - start;
                averageTime += diff.count();
            }
            saveImage("img/" + imageSize + "x" + imageSize + " result.png", image, size);
            cout << "Plain operation: " << averageTime * 1000 / timesToRepeat << "\n";

            // OMP
            averageTime = 0;
            for (int i = 0; i < timesToRepeat; i++) {
                start = chrono::system_clock::now();
                image = openMP(img1, img2, operations[operationNumber]);
                end = chrono::system_clock::now();
                diff = end - start;
                averageTime += diff.count();
            }
            cout << "OMP operation: " << averageTime * 1000 / timesToRepeat << "\n";

            // Vectorization
            averageTime = 0;
            for (int i = 0; i < timesToRepeat; i++) {
                start = chrono::system_clock::now();
                image = vectorize(img1, img2);
                end = chrono::system_clock::now();
                diff = end - start;
                averageTime += diff.count();
            }
            cout << "Vectorize operation: " << averageTime * 1000 / timesToRepeat << "\n";

            // Vectorization + OMP
            averageTime = 0;
            for (int i = 0; i < timesToRepeat; i++) {
                start = chrono::system_clock::now();
                image = vectorizeOMP(img1, img2);
                end = chrono::system_clock::now();
                diff = end - start;
                averageTime += diff.count();
            }
            cout << "Vectorization + OMP operation: " << averageTime * 1000 / timesToRepeat << "\n";

            saveImage("img/" + imageSize + "x" + imageSize + " result.png", image, size);
        }
        cout << "Write the number of operation and press Enter: ";
    }

    return 0;
}