#include "thread_utils.h"
#include "mem_pool.h"
#include "lock_free_queue.h"

using namespace Common;
using namespace std::literals::chrono_literals;

/** test threads */
auto dummyFunction(int a, int b, bool sleep)
{
    std::cout << "Dummy function(" << a << "," << b << ")" << std::endl;
    std::cout << "Dummy output = " << a * b << std::endl;

    if (sleep)
    {
        std::cout << "dummyFunction sleeping ..." << std::endl;
        std::this_thread::sleep_for(5s);
    }
    std::cout << "dummyFunction done. " << std::endl;
}

struct MyStruct
{
    int d_[3];
};

/** test lock free queue */
auto consumeFunction(LFQueue<MyStruct>* lfq)
{
    std::this_thread::sleep_for(5s);

    while (lfq -> size())
    {
        const auto d = lfq -> getNextToRead();
        lfq -> updateWriteIndex();

        std::cout << "consumeFunction read elem: "
            << d -> d_[0] << ", "
            << d -> d_[1] << ", "
            << d -> d_[2] << " lfq-size: " << lfq -> size() << std::endl;

        std::this_thread::sleep_for(1s);
    }
    std::cout << "consumeFunction exiting." << std::endl;
}

int main(int, char **)
{
    auto t1 = createAndStartThread(-1, "dummyFunction1", dummyFunction, 10, 30, false);
    auto t2 = createAndStartThread( 1, "dummyFunction2", dummyFunction, 20, 51, true );

    std::cout << "main waiting for threads to be done." << std::endl;
    t1 -> join();
    t2 -> join();

    MemPool<double> prim_pool(5);
    MemPool<MyStruct> struct_pool(5);

    for (auto i = 0; i < 5; i++)
    {
        auto p_ret = prim_pool.allocate(i);
        auto s_ret = struct_pool.allocate(MyStruct{i, i + 1, i + 2});

        std::cout << "prim elem: " << *p_ret << " allocated at: " << p_ret << std::endl;
        std::cout << "struct elem: "
            << s_ret ->d_[0] << ", "
            << s_ret ->d_[1] << ", "
            << s_ret ->d_[2] << " allocated at: "
            << s_ret << std::endl;
        if (i % 5 == 0)
        {
            std::cout << "deallocating prim elem: " << *p_ret << " from: " << p_ret << std::endl;
            std::cout << "deallocating struct elem: "
                << s_ret ->d_[0] << ", "
                << s_ret ->d_[1] << ", "
                << s_ret ->d_[2] << " from: " << s_ret << std::endl;

            prim_pool.deallocate(p_ret);
            struct_pool.deallocate(s_ret);
        }
    }

    LFQueue<MyStruct> lfq(20);

    auto ct = createAndStartThread(-1, "", consumeFunction, &lfq);

    for ( auto i = 0; i < 50; i++)
    {
        const MyStruct d{1, i * 10, i * 100};
        *(lfq.getNextToWriteTo()) = d;
        lfq.updateWriteIndex();

        std::cout << "main constructed elem: "
            << d.d_[0] << ", "
            << d.d_[1] << ", "
            << d.d_[2] << " lfq-size"
            << lfq.size() << std::endl;

        std::this_thread::sleep_for(1s);
    }
    ct -> join();
    std::cout << "main exiting." << std::endl;

    return 0;
}