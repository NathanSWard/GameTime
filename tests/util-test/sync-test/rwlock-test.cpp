#include <ut.hpp>
#include <util/sync/rwlock.hpp>
#include <thread>

using namespace boost::ut;

void rwlock_test()
{
    "[RwLock]"_test = [] {
        auto rwlock = RwLock<int>::create(0);

        should("allow only 1 thread to write") = [&] {
            auto write = rwlock.write();
            auto t = std::thread([&] {
                expect(!rwlock.try_write().has_value());
                expect(!rwlock.try_read().has_value());
                });
            t.join();
        };

        should("allow multiple threads to read") = [&] {
            auto read = rwlock.read();
            auto t = std::thread([&] {
                expect(!rwlock.try_write().has_value());
                expect(rwlock.try_read().has_value());
                });
            t.join();
        };
    };
}