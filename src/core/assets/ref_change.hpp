#pragma once

#include <core/assets/handle_id.hpp>
#include <util/common.hpp>
#include <util/sync/mpmc.hpp>

struct RefChange
{
    enum Type : bool { Increment, Decrement } type = Increment;
    HandleId id = HandleId(AssetPathId(0));

    [[nodiscard]] constexpr static auto increment(HandleId const id) noexcept -> RefChange
    {
        return RefChange{ .type = Increment, .id = id };
    }

    [[nodiscard]] constexpr static auto decrement(HandleId const id) noexcept -> RefChange
    {
        return RefChange{ .type = Decrement, .id = id };
    }
};

struct RefChangeChannel
{
    Sender<RefChange> sender;
    Receiver<RefChange> receiver;

    RefChangeChannel(Sender<RefChange> sender, Receiver<RefChange> receiver)
        : sender(MOV(sender))
        , receiver(MOV(receiver))
    {}

    static auto create() -> RefChangeChannel
    {
        auto [send, recv] = mpmc_channel<RefChange>();
        return RefChangeChannel(MOV(send), MOV(recv));
    }
};