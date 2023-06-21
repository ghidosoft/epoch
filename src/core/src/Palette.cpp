#include "Palette.h"

namespace epoch
{
    Palette::Palette(std::span<Color> colors)
    {
        m_palette.resize(colors.size());
        m_palette.assign(colors.begin(), colors.end());
    }

    Color Palette::map(const std::size_t index) const
    {
        return m_palette[index];
    }

    std::size_t Palette::size() const
    {
        return m_palette.size();
    }
}
