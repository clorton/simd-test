// simdTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdint>
#include <chrono>
#include <random>

#include <intrin.h>

class Individual
{
public:
    Individual(uint32_t idx, float inf = 0.0f) : index(idx), infectivity(inf) {}

    uint32_t    index;
    float infectivity;
    int32_t fluff[256]; // Bummer, ;), we'll need more than a page of memory for each of these...
};

#if !defined(_DEBUG)
#define POPULATION  (1 << 20)   // 2^20 entries ~ 10^6
#else
#define POPULATION  (1 << 10)   // 2^10 entries ~ 10^3
#endif

void Attenuate(std::vector<float>& data);
float Sum(std::vector<float>& data);

int main()
{
    std::mt19937 prng(20160602);
    std::uniform_real<float> dist(0.0f, 1.0f);

    std::vector<float> source(POPULATION);
    for (auto& entry : source)
    {
        entry = dist(prng);
    }

    std::vector<Individual*> population(POPULATION);
    uint32_t index = 0;
    for (auto& individual : population)
    {
        individual = new Individual(index, source[index]);
        ++index;
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    // Attenuate infectivity by 10% 10 times.
    for (uint32_t i = 0; i < 10; ++i)
    {
        for (auto individual : population)
        {
            individual->infectivity *= 0.9f;
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    // Sum remaining infectivity.
    float total = 0.0f;
    for (auto individual : population)
    {
        total += individual->infectivity;
    }

    auto t3 = std::chrono::high_resolution_clock::now();

    printf("Total infectivity: %lf\n", total);
    double elapsed = uint64_t((t2 - t1).count()) * 1000 * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den);
    printf("Attenuate (ms): %lf\n", elapsed);
    elapsed = uint64_t((t3 - t2).count()) * 1000 * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den);
    printf("Sum (ms):       %lf\n", elapsed);

    std::vector<float> data(source);

    t1 = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < 10; ++i)
    {
        Attenuate(data);
    }
    t2 = std::chrono::high_resolution_clock::now();
    total = Sum(data);
    t3 = std::chrono::high_resolution_clock::now();

    printf("Total infectivity: %lf\n", total);
    elapsed = uint64_t((t2 - t1).count()) * 1000 * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den);
    printf("Attenuate (ms): %lf\n", elapsed);
    elapsed = uint64_t((t3 - t2).count()) * 1000 * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den);
    printf("Sum (ms):       %lf\n", elapsed);

    return 0;
}

#define SSE

void Attenuate(std::vector<float>& data)
{
    float factor = 0.9f;
    float* ptr = data.data();
    uint32_t count = uint32_t(data.size()) / 8;
#ifdef SSE
    auto f = _mm_load_ps1(&factor);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto a = _mm_load_ps(ptr);
        auto b = _mm_load_ps(ptr + 4);
        a = _mm_mul_ps(f, a);
        b = _mm_mul_ps(f, b);
        _mm_store_ps(ptr, a);
        _mm_store_ps(ptr + 4, b);
        ptr += 8;
    }
#else
    auto f = _mm256_broadcast_ss(&factor);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto a = _mm256_load_ps(ptr);
        a = _mm256_mul_ps(f, a);
        _mm256_store_ps(ptr, a);
        ptr += 8;
    }
#endif
}

float Sum(std::vector<float>& data)
{
    float* ptr = data.data();
    uint32_t count = uint32_t(data.size()) / 8;
#ifdef SSE
    auto acc1 = _mm_setzero_ps();
    auto acc2 = _mm_setzero_ps();
    for (uint32_t i = 0; i < count; ++i)
    {
        auto a = _mm_load_ps(ptr);
        auto b = _mm_load_ps(ptr + 4);
        acc1 = _mm_add_ps(a, acc1);
        acc2 = _mm_add_ps(b, acc2);
        ptr += 8;
    }
    acc1 = _mm_add_ps(acc2, acc1);
    acc1 = _mm_hadd_ps(acc1, acc1);
    acc1 = _mm_hadd_ps(acc1, acc1);
    __declspec(align(16)) float total = 3.14159265f;
    _mm_store_ps1(&total, acc1);
#else
    auto acc1 = _mm256_setzero_ps();
    for (uint32_t i = 0; i < count; ++i)
    {
        auto a = _mm256_load_ps(ptr);
        acc1 = _mm256_add_ps(a, acc1);
        ptr += 8;
    }
    acc1 = _mm256_hadd_ps(acc1, acc1);
    acc1 = _mm256_hadd_ps(acc1, acc1);
    float temp[8];
    _mm256_store_ps(temp, acc1);
    float total = temp[0] + temp[4];
#endif

    return total;
}
