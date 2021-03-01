#include <ut.hpp>
#include <core/input/input.hpp>

using namespace boost::ut;

enum class DummyInput {
    Input1,
    Input2,
};

void input_test()
{
    "[Input]"_test = [] {
        {
            auto input = Input<DummyInput>();

            input.press(DummyInput::Input1);
            input.press(DummyInput::Input2);

            should("show pressed & just_pressed") = [&] {
                expect(input.just_pressed(DummyInput::Input1));
                expect(input.just_pressed(DummyInput::Input2));
                expect(input.pressed(DummyInput::Input1));
                expect(input.pressed(DummyInput::Input2));
            };

            input.update();

            should("remove just_pressed status") = [&] {
                expect(!input.just_pressed(DummyInput::Input1));
                expect(!input.just_pressed(DummyInput::Input2));
                expect(input.pressed(DummyInput::Input1));
                expect(input.pressed(DummyInput::Input2));
            };

            input.release(DummyInput::Input1);
            input.release(DummyInput::Input2);

            should("show not pressed & just_released") = [&] {
                expect(input.just_released(DummyInput::Input1));
                expect(input.just_released(DummyInput::Input2));
                expect(!input.pressed(DummyInput::Input1));
                expect(!input.pressed(DummyInput::Input2));
            };

            input.update();

            should("remove just_released status") = [&] {
                expect(!input.just_released(DummyInput::Input1));
                expect(!input.just_released(DummyInput::Input2));
            };
        }

        // input for resettings
        auto input = Input<DummyInput>();

        input.press(DummyInput::Input1);
        input.release(DummyInput::Input2);

        input.reset(DummyInput::Input1);
        input.reset(DummyInput::Input2);

        should("not show just_(pressed/released)") = [&] {
            expect(!input.just_pressed(DummyInput::Input1));
            expect(!input.pressed(DummyInput::Input1));

            expect(!input.just_released(DummyInput::Input2));
        };
    };
}