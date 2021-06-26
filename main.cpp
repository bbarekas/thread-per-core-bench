#include <iostream>
#include <thread>
#include <atomic>
#include <benchmark/benchmark.h>
#include "rust-mt-lib.h"


using namespace std;

// Pad extra bytes so that each variable will take place of the whole cache line
struct alignas(64) padded_atomic_int {
    atomic<int> value;

    padded_atomic_int() { value = 0; }
};


//
void helloFunction() {
    std::cout << "Hello C++11 from function." << endl;
}

//
class HelloFunctionObject {
public:
    void operator()() const {
        std::cout << "Hello C++11 from a function object." << endl;
    }
};


//
void update(atomic<int> &a) {
    for (int i = 0; i < 10000000; ++i) {
        ++a;
    }
}

//
void single_thread() {
    atomic<int> a, b, c, d;
    a = b = c = d = 0;

    // Update 4 variables sequentially
    update(a);
    update(b);
    update(c);
    update(d);
}

//
void multi_thread_false() {
    atomic<int> a, b, c, d; // enforce false-sharing
    a = b = c = d = 0;

    // Assign each atomic<int> to each thread
    std::thread t1([&]() { update(a); });
    std::thread t2([&]() { update(b); });
    std::thread t3([&]() { update(c); });
    std::thread t4([&]() { update(d); });

    // Join the 4 threads
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

//
void multi_thread_padding() {
    padded_atomic_int a{};
    padded_atomic_int b{};
    padded_atomic_int c{};
    padded_atomic_int d{};

    // Assign each thread one aligned atomic int type
    thread t1([&]() { update(a.value); });
    thread t2([&]() { update(b.value); });
    thread t3([&]() { update(c.value); });
    thread t4([&]() { update(d.value); });

    // Join threads
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

//
void sub_optimal_scheduling() {
    padded_atomic_int a{};
    padded_atomic_int b{};

    // Create four threads and use lambda to launch work
    thread t0([&]() { update(a.value); });
    thread t1([&]() { update(a.value); });
    thread t2([&]() { update(b.value); });
    thread t3([&]() { update(b.value); });

    // Join threads
    t0.join();
    t1.join();
    t2.join();
    t3.join();
}

void optimal_scheduling() {
    padded_atomic_int a{};
    padded_atomic_int b{};

    // Create cpu sets for threads 0,1 and 2,3
    cpu_set_t cpu_set_1;
    cpu_set_t cpu_set_2;

    // Clears set, so that it contains no CPUs.
    CPU_ZERO(&cpu_set_1);
    CPU_ZERO(&cpu_set_2);

    // Add the CPU cores we want to pin the threads to
    CPU_SET(0, &cpu_set_1);
    CPU_SET(1, &cpu_set_2);

    // Create 4 threads
    pthread_t *threads = (pthread_t *) malloc(4 * sizeof(pthread_t));

    // Init thread attr for 2 cpu sets
    pthread_attr_t attr1, attr2;
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);

    // Bind thread 0 and thread 1 to core 0
    pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &cpu_set_1);
    pthread_create(&threads[0], &attr1,
                   [](void *a) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(a)).value);
                       return 0;
                   }, &a);
    pthread_create(&threads[1], &attr1,
                   [](void *a) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(a)).value);
                       return 0;
                   }, &a);

    // Bind thread 2 and thread 3 to core 1
    pthread_attr_setaffinity_np(&attr2, sizeof(cpu_set_t), &cpu_set_2);
    pthread_create(&threads[2], &attr2,
                   [](void *b) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(b)).value);
                       return 0;
                   }, &b);
    pthread_create(&threads[3], &attr2,
                   [](void *b) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(b)).value);
                       return 0;

                   }, &b);

    // Join threads
    void *status;
    for (long t = 0; t < 4; ++t)
        pthread_join(threads[t], &status);
}

//
void percore_scheduling() {
    padded_atomic_int a{};
    padded_atomic_int b{};
    padded_atomic_int c{};
    padded_atomic_int d{};

    // Create cpu sets for threads 0,1 and 2,3
    cpu_set_t cpu_set_1;
    cpu_set_t cpu_set_2;
    cpu_set_t cpu_set_3;
    cpu_set_t cpu_set_4;

    // Clears set, so that it contains no CPUs.
    CPU_ZERO(&cpu_set_1);
    CPU_ZERO(&cpu_set_2);
    CPU_ZERO(&cpu_set_3);
    CPU_ZERO(&cpu_set_4);

    // Add the CPU cores we want to pin the threads to
    CPU_SET(4, &cpu_set_1);
    CPU_SET(1, &cpu_set_2);
    CPU_SET(2, &cpu_set_3);
    CPU_SET(3, &cpu_set_4);

    // Create 4 threads
    pthread_t *threads = (pthread_t *) malloc(4 * sizeof(pthread_t));

    // Init thread attr for 2 cpu sets
    pthread_attr_t attr1, attr2, attr3, attr4;
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_attr_init(&attr3);
    pthread_attr_init(&attr4);

    // Bind thread 0 to core 0
    pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &cpu_set_1);
    pthread_create(&threads[0], &attr1,
                   [](void *a) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(a)).value);
                       return 0;
                   }, &a);

    // Bind thread 1 to core 1
    pthread_attr_setaffinity_np(&attr2, sizeof(cpu_set_t), &cpu_set_2);
    pthread_create(&threads[1], &attr2,
                   [](void *b) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(b)).value);
                       return 0;
                   }, &b);

    // Bind thread 2 to core 2
    pthread_attr_setaffinity_np(&attr3, sizeof(cpu_set_t), &cpu_set_3);
    pthread_create(&threads[2], &attr3,
                   [](void *c) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(c)).value);
                       return 0;
                   }, &c);

    // Bind thread 3 to core 3
    pthread_attr_setaffinity_np(&attr4, sizeof(cpu_set_t), &cpu_set_4);
    pthread_create(&threads[3], &attr4,
                   [](void *d) -> void * {
                       update((*reinterpret_cast<padded_atomic_int *>(d)).value);
                       return 0;
                   }, &d);

    // Join threads
    void *status;
    for (long t = 0; t < 4; ++t)
        pthread_join(threads[t], &status);
}

//
void various_thread() {

    // thread executing helloFunction
    std::thread t1(helloFunction);

    // thread executing helloFunctionObject
    HelloFunctionObject helloFunctionObject;
    std::thread t2(helloFunctionObject);

    // thread executing lambda function
    std::thread t3([] { std::cout << "Hello C++11 from lambda function." << endl; });

    // ensure that t1, t2 and t3 have finished before main terminates
    t1.join();
    t2.join();
    t3.join();
}

//
void print() {
    atomic<int> a, b, c, d;

    // Return addresses of atomic variables
    cout << "Address of a is: " << &a << '\n';
    cout << "Address of b is: " << &b << '\n';
    cout << "Address of c is: " << &c << '\n';
    cout << "Address of d is: " << &d << '\n';

    padded_atomic_int pa{};
    padded_atomic_int pb{};
    padded_atomic_int pc{};
    padded_atomic_int pd{};

    // Return addresses of atomic variables
    cout << "Address of pa is: " << &pa << '\n';
    cout << "Address of pb is: " << &pb << '\n';
    cout << "Address of pc is: " << &pc << '\n';
    cout << "Address of pd is: " << &pd << '\n';

}

#if 1

static void bench_singleT(benchmark::State &state) {
    for (auto _ : state)
        single_thread();
}
//BENCHMARK(bench_singleT);
BENCHMARK(bench_singleT)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_false(benchmark::State &state) {
    for (auto _ : state)
        multi_thread_false();
}
//BENCHMARK(bench_multiT);
BENCHMARK(bench_multiT_false)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_padded(benchmark::State &state) {
    for (auto _ : state)
        multi_thread_padding();
}
//BENCHMARK(bench_multiT);
BENCHMARK(bench_multiT_padded)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_subopt(benchmark::State &state) {
    for (auto _ : state)
        sub_optimal_scheduling();
}
//BENCHMARK(bench_multiT);
BENCHMARK(bench_multiT_subopt)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_optimal(benchmark::State &state) {
    for (auto _ : state)
        optimal_scheduling();
}
//BENCHMARK(bench_multiT);
BENCHMARK(bench_multiT_optimal)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_percore(benchmark::State &state) {
    for (auto _ : state)
        percore_scheduling();
}
BENCHMARK(bench_multiT_percore)->UseRealTime()->Unit(benchmark::kMillisecond);

// Rust

static void bench_rust_hello(benchmark::State &state) {
    for (auto _ : state)
        print_hello();
}
BENCHMARK(bench_rust_hello)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_singleT_rs(benchmark::State &state) {
    for (auto _ : state)
        single_thread_rs();
}
BENCHMARK(bench_singleT_rs)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_rs(benchmark::State &state) {
    for (auto _ : state)
        multi_thread_rs();
}
BENCHMARK(bench_multiT_rs)->UseRealTime()->Unit(benchmark::kMillisecond);

static void bench_multiT_arc_rs(benchmark::State &state) {
    for (auto _ : state)
        multi_thread_arc_rs();
}
BENCHMARK(bench_multiT_arc_rs)->UseRealTime()->Unit(benchmark::kMillisecond);


// Run the benchmark
BENCHMARK_MAIN();

#else

//
int main() {
    std::cout << "Hello, World!" << std::endl;

    printf("Size of atomic<int>: %lu\n", sizeof(atomic<int>));
    printf("Size of padded_atomic_int: %lu\n", sizeof(padded_atomic_int));

    // Sum
    atomic<int> sum;
    update(sum);
    std::cout << "Sum = " << sum << std::endl;

    // Single-Threaded
    single_thread();

    // Multi-Threaded False
    multi_thread_false();

    // Multi-Threaded Padded
    multi_thread_padding();

    // Multi-Threaded Padded
    multi_thread_padding();

    // Multi-Threaded Padded
    multi_thread_padding();

    // Various Threads
    various_thread();

    // Print
    //print();

    // Calling rust library
    print_hello();

    return 0;
}
#endif