#pragma once

#include <tl/optional.hpp>

#include <core/assets/handle.hpp>
#include <core/game/events.hpp>
#include <util/common.hpp>
#include <util/containers/hash.hpp>

template <typename T>
struct AssetEvent
{
    enum Type { Created, Modified, Removed } type = Created;
    Handle<T> handle;

    template <typename H>
    static auto created(H&& h) -> AssetEvent
    {
        return AssetEvent{ .type = Created, .handle = FWD(h) };
    }

    template <typename H>
    static auto modified(H&& h) -> AssetEvent
    {
        return AssetEvent{ .type = Modified, .handle = FWD(h) };
    }

    template <typename H>
    static auto removed(H&& h) -> AssetEvent
    {
        return AssetEvent{ .type = Removed, .handle = FWD(h) };
    }
};

template <typename T>
class AssetsBase
{
protected:
    HashMap<HandleId, T> m_assets;
    std::vector<AssetEvent<T>> m_events;
    Sender<RefChange> m_sender;

public:
    AssetsBase(Sender<RefChange> sender) noexcept
        : m_sender(MOV(sender))
    {}

    AssetsBase(AssetsBase&&) noexcept = default;
    AssetsBase& operator=(AssetsBase&&) noexcept = default;

    AssetsBase(AssetsBase const&) = delete;
    AssetsBase& operator=(AssetsBase const&) = delete;

    auto get_handle(HandleId const id) -> Handle<T>
    {
        return Handle<T>::strong(id, m_sender);
    }

    template <typename... Args>
    auto add_asset(Args&&... args) -> Handle<T>
    {
        auto const id = HandleId::random<T>();
        auto const [iter, inserted] = m_assets.insert_or_assign(id, T(FWD(args)...));
        UNUSED(inserted);

        m_events.push_back(AssetEvent<T>::created(Handle<T>::weak(id)));
        return get_handle(id);
    }

    template <typename... Args>
    void set_asset(HandleId const id, Args&&... args)
    {
        auto const [iter, inserted] = m_assets.insert_or_assign(id, T(FWD(args)...));

        if (inserted) {
            m_events.push_back(AssetEvent<T>::created(Handle<T>::weak(id)));
        }
        else {
            m_events.push_back(AssetEvent<T>::modified(Handle<T>::weak(id)));
        }
    }

    [[nodiscard]] auto contains_asset(HandleId const id) const noexcept -> bool
    {
        return m_assets.contains(id);
    }

    // NOTE: will *not* send an AssetEvent.
    [[nodiscard]] auto get_mut_asset_untracked(HandleId const id) -> tl::optional<T&>
    {
        if (auto const iter = m_assets.find(id); iter != m_assets.end()) {
            return tl::make_optional<T&>(iter->second);
        }
        return {};
    }

    // NOTE: Will send an AssetEvent::Modified event.
    [[nodiscard]] auto get_mut_asset(HandleId const id) -> tl::optional<T&>
    {
        if (auto const iter = m_assets.find(id); iter != m_assets.end()) {

            m_events.push_back(AssetEvent<T>::modified(Handle<T>::weak(id)));

            return tl::make_optional<T&>(iter->second);
        }
        return {};
    }

    [[nodiscard]] auto get_asset(HandleId const id) const -> tl::optional<T const&>
    {
        if (auto const iter = m_assets.find(id); iter != m_assets.end()) {
            return tl::make_optional<T const&>(iter->second);
        }
        return {};
    }

    auto remove_asset(HandleId const id) -> tl::optional<T>
    {
        if (auto const iter = m_assets.find(id); iter != m_assets.end()) {
            auto value = MOV(iter->second);
            m_assets.erase(iter);

            m_events.push_back(AssetEvent<T>::removed(Handle<T>::weak(id)));

            return tl::make_optional<T>(MOV(value));
        }
        return {};
    }

    [[nodiscard]] auto begin() { return m_assets.begin(); }
    [[nodiscard]] auto end() { return m_assets.end(); }
    [[nodiscard]] auto begin() const { return m_assets.begin(); }
    [[nodiscard]] auto end() const { return m_assets.end(); }

    void clear() { m_assets.clear(); }
    void reserve(std::size_t const size) { m_assets.reserve(size); }
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_assets.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return m_assets.empty(); }

    void update_events(Events<AssetEvent<T>>& events)
    {
        events.send_batch(std::make_move_iterator(m_events.begin()), std::make_move_iterator(m_events.end()));
        std::vector<AssetEvent<T>>().swap(m_events);
    }
};

template <typename T>
class Assets : public AssetsBase<T> {};

template <typename T>
void asset_event_system(EventWriter<AssetEvent<T>> events, Resource<Assets<T>> assets)
{
    assets->update_events(*events);
}