#ifndef SRC_EPOCH_CORE_PALETTE_H_
#define SRC_EPOCH_CORE_PALETTE_H_

#include <span>
#include <vector>

#include "Color.h"

namespace epoch
{
    class Palette final
    {
    public:
        explicit Palette(std::span<Color> colors);

        [[nodiscard]] Color map(std::size_t index) const;
        [[nodiscard]] std::size_t size() const;

    private:
        std::vector<Color> m_palette{};
    };
}

#endif
