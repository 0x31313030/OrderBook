#pragma once
#include <cstddef>
#include <atomic>

/**
 * @brief Generates unique IDs sequentially. It is thread safe using atomics.
 * NOTE: The ID can overflow so depending on use, it might have to be reset
 * in a controlled way by calling 'Reset()'.
 */
class UniqueIDGenerator 
{
public:
    UniqueIDGenerator() : m_id(0) {}

    /**
     * @brief Generates a unique id sequentially. Is thread safe.
     */
    size_t GenerateID()
    {
        return m_id.fetch_add(1);
    }

    /**
     * @brief Resets id to 0.
     * 
     */
    void Reset()
    {
        m_id.store(0);
    }

private:
    std::atomic<size_t> m_id;
};