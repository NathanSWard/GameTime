#include <ut.hpp>
#include <util/memory/rc.hpp>

using namespace boost::ut;

bool destroyed = false;
struct Test
{
    ~Test() { destroyed = true; }
};

void rc_test()
{
    "[Rc]"_test = [] {
        destroyed = false;

        auto const weak = [] {
            auto rc = Rc<Test>::create();
            expect(rc.strong_count() == 1);
            expect(rc.weak_count() == 0);

            auto weak = rc.downgrade();
            expect(rc.strong_count() == 1);
            expect(rc.weak_count() == 1);

            auto rc2 = weak.upgrade();
            expect((rc2.has_value()) >> fatal);

            expect(rc.strong_count() == 2);
            expect(rc.weak_count() == 1);

            expect(!destroyed);

            return weak;
        }();

        expect(destroyed);
        expect(!weak.upgrade().has_value());
    };
}