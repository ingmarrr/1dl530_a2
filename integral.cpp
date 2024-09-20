#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <vector>
#include <time.h>

const char* USAGE =
    "Usage: ./integrate [-h] <num_threads> <num_trapezes>\n"
    "  -h            : Print this help message and exit\n"
    "  num_threads   : Number of threads to use\n"
    "  num_trapezes  : Number of trapezes for integration\n";

#define error(...) \
    std::cerr << "[error] " << __VA_ARGS__ << std::endl; \
    std::cerr << USAGE << std::endl; \
    exit(1); \

auto erange(uint64_t start, uint64_t end) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i < end; ++i)
    {
        out.push_back(i);
    }

    return out;
}

auto irange(uint64_t start, uint64_t end) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i <= end; ++i) 
    {
        out.push_back(i);
    }

    return out;
}

auto f(double x) -> double
{
    return 4.0 / (1.0 + x * x);
}

auto seq_integrate(int n) -> double
{
    const double a  = 0.0;
    const double b  = 1.0;
    auto width      = (b - a) / n;
    auto sum        = 0.0;

    for (auto xi : irange(1, n))
    {
        sum += f(a + xi * width);
    }
    return width * sum / n;
}

auto partial(
    double& sum, 
    const uint64_t index, 
    const double a, 
    const double width, 
    const uint64_t start, 
    const uint64_t end
) {
    auto out = 0.0;
    for (auto xi = start; xi <= end; ++xi)
    {
        out += f(a + xi * width);
    }
    sum = out;
}

auto par_integreate(int n_threads, int n_traps) -> double
{
    const double a  = 0.0;
    const double b  = 1.0;
    auto width      = (b - a) / n_traps;

    std::vector<double> partials(n_threads);
    std::vector<std::thread> workers(n_threads);

    auto chunks     = div(n_traps, n_threads);

    auto start = 1;
    for (int tix : erange(0, n_threads)) 
    {
        auto end = start + chunks.quot - 1 + (tix < chunks.rem ? 1 : 0);
        workers[tix] = std::thread(
            partial, 
            std::ref(partials[tix]),
            tix, a, width, start, end
        );
        start = end + 1;
    }

    for (auto t : erange(0, n_threads))
    {
        workers[t].join();
    }

    auto total = std::accumulate(partials.begin(), partials.end(), 0.0);
    return width * total / n_traps;
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        if (strncmp(argv[1], "--help", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)
        {
            std::cout << USAGE << std::endl;
            exit(0);
        }
        else
    {
            error("invalid argument, expected [--help | -h], found " << argv[1]);
        }
    }

    if (argc != 3)
    {
        error("expected two arguments, found " << argc);
    }

    auto n_threads = std::stoll(argv[1]);
    /* auto n_traps   = std::stoll(argv[2]); */


    std::cout << "Nr of Threads: " << n_threads << std::endl;

    std::cout << std::setprecision(15);
    const double PI = 3.14159265358979323846;

    std::mutex io_mutex{};

    for (int n = 1; n <= 100000000; n *= 10) 
    {
        io_mutex.lock();

        auto seq_start  = (double) clock();
        seq_start       = seq_start / CLOCKS_PER_SEC;
        auto seq_result = seq_integrate(n);
        auto seq_diff    = ( ((double) clock()) / CLOCKS_PER_SEC) - seq_start;
        auto seq_err   = std::abs(seq_result - PI) / PI;

        std::cout 
            << "Trapezoids [seq]: " << n << std::endl
            << " Result: " << seq_result << std::endl
            << " Accuracy: " << seq_err << std::endl
            << " The elapsed time is: "<< seq_diff << " in seconds"<< std::endl;

        auto par_start  = (double) clock();
        par_start       = par_start / CLOCKS_PER_SEC;
        auto par_result = par_integreate(n_threads, n);
        auto par_diff    = ( ((double) clock()) / CLOCKS_PER_SEC) - par_start;
        auto par_err   = std::abs(par_result - PI) / PI;

        std::cout 
            << "Trapezoids [par]: " << n << std::endl
            << " Result: " << par_result << std::endl
            << " Accuracy: " << par_err << std::endl
            << " The elapsed time is: "<< par_diff << " in seconds"<< std::endl;

        std::cout << std::endl << std::endl;
        io_mutex.unlock();
    }

    return 0;
}
