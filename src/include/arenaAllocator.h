#pragma once

class ArenaAllocator
{
private:
    size_t m_Size;
    std::byte *m_Buffer;
    std::byte *m_Offset;

public:
    inline explicit ArenaAllocator(size_t bytes) : m_Size(bytes)
    {
        m_Buffer = static_cast<std::byte *>(malloc(m_Size));
        m_Offset = m_Buffer;
    }

    template<typename T>
    inline T* allocate(){
        void* offset = m_Offset;
        m_Offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    inline ArenaAllocator(const ArenaAllocator &arena) = delete;

    inline ArenaAllocator operator=(const ArenaAllocator &arena) = delete;


    inline ~ArenaAllocator()
    {
        free(m_Buffer);
    }
};
