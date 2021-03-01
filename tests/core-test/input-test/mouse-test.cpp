#include <ut.hpp>
#include <core/input/mouse.hpp>

using namespace boost::ut;

void mouse_test()
{
    "[MouseInput]"_test = [] {
        using hash_t = std::hash<MouseButton>;

        auto const left = hash_t{}(MouseButton(MouseButton::Type::Left));
        auto const right = hash_t{}(MouseButton(MouseButton::Type::Right));
        auto const middle = hash_t{}(MouseButton(MouseButton::Type::Middle));
        auto const other_1 = hash_t{}(MouseButton(MouseButton::OtherButton{ 1 }));
        auto const other_2 = hash_t{}(MouseButton(MouseButton::OtherButton{ 2 }));

        expect(left != right && left != middle && left != other_1 && left != other_2);
        expect(right != middle && right != other_1 && right != other_2);
        expect(middle != other_1 && middle != other_2);
        expect(other_1 != other_2);
    };
}