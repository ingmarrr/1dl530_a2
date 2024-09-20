#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>
#include <time.h>

double time1, timedif;

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
    for (auto tix : erange(0, n_threads)) 
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
    auto n_traps   = std::stoll(argv[2]);

    time1 = (double) clock();            /* get initial time */
    time1 = time1 / CLOCKS_PER_SEC;      /*    in seconds    */

    // printf("Nr of Threads: %llu\nNr Of Trapezes: %llu\n", n_threads, n_traps);

    std::cout << std::setprecision(15);
    const double PI = 3.14159265358979323846;

    for (int n = 1; n <= 100000000; n *= 10) 
    {
        //double result = par_integreate(n_threads, n);
        double result = seq_integrate(n);
        double relative_error = std::abs(result - PI) / PI;

        timedif = ( ((double) clock()) / CLOCKS_PER_SEC) - time1;
        
        std::cout 
            << "Trapezoids: " << std::setw(9) << n 
            << " Result: " << result 
            << " Relative Error: " << relative_error
            << " The elapsed time is: "<< timedif << " in seconds"<< std::endl;
    }


    return 0;
}
