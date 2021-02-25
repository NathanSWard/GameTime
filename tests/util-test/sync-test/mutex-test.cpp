#include <ut.hpp>
#include <util/sync/mutex.hpp>
#include <thread>

using namespace boost::ut;

void mutex_test()
{
    "[Mutex]"_test = [] {
        auto mutex = make_mutex<int>(0);

        should("prevent thread from taking lock") = [&] {
            auto lock = mutex.lock();

            auto t = std::thread([&mutex] {
                expect(!mutex.try_lock().has_value());
                });
            t.join();

            ++(*lock);
        };

        should("allow thread to lock") = [&] {
            auto t = std::thread([&mutex] {
                auto lock = mutex.try_lock();
                expect((lock.has_value()) >> fatal);
                expect(**lock == 1);
                ++(**lock);
                });
            t.join();
        };

        auto lock = mutex.lock();
        expect(*lock == 2);
    };
}