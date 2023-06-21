#include "Palette.h"

namespace epoch
{
    Color Palette::map(const std::size_t index) const
    {
        return m_palette[index];
    }

    std::size_t Palette::size() const
    {
        return m_palette.size();
    }
}
