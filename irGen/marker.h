#ifndef JIT_AOT_COURSE_MARKER_H_
#define JIT_AOT_COURSE_MARKER_H_

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

namespace ir {
using Marker = uint32_t;

enum class MarkersConstants : uint32_t {
    BIT_SIZE = sizeof(Marker) * 8,
    BIT_LENGTH = 2,
    MAX_MARKERS = 1U << BIT_LENGTH,
    INDEX_MASK = MAX_MARKERS - 1U,
    MAX_VALUE = (1U << (BIT_SIZE - BIT_LENGTH)) - 1U,
    UNDEF_VALUE = 0,
};

class MarkerManager {
  public:
    MarkerManager() = default;
    MarkerManager(const MarkerManager &) = delete;
    MarkerManager &operator=(const MarkerManager &) = delete;
    MarkerManager(MarkerManager &&) = delete;
    MarkerManager &operator=(MarkerManager &&) = delete;
    virtual ~MarkerManager() = default;

    Marker GetNewMarker() {
        assert(currentIndex <
               static_cast<uint8_t>(MarkersConstants::MAX_VALUE));
        ++currentIndex;
        for (uint32_t i = 0; i < markersSlots.size(); ++i) {
            if (!markersSlots[i]) {
                Marker mark = (currentIndex << static_cast<uint8_t>(
                                   MarkersConstants::BIT_LENGTH)) |
                              i;
                markersSlots[i] = true;
                return mark;
            }
        }
        std::cerr << "free marker slot not found" << std::endl;
        return 0;
    }
    void ReleaseMarker(Marker mark) {
        size_t index =
            mark & static_cast<uint8_t>(MarkersConstants::INDEX_MASK);
        markersSlots[index] = false;
    }

  private:
    size_t currentIndex = 0;
    std::bitset<static_cast<uint8_t>(MarkersConstants::MAX_MARKERS)>
        markersSlots{false};
};

class Markable {
  public:
    Markable() = default;
    Markable(const Markable &) = delete;
    Markable &operator=(const Markable &) = delete;
    Markable(Markable &&) = delete;
    Markable &operator=(Markable &&) = delete;
    virtual ~Markable() noexcept = default;

    bool SetMarker(Marker mark) {
        auto value = mark >> static_cast<uint8_t>(MarkersConstants::BIT_LENGTH);
        size_t index =
            mark & static_cast<uint8_t>(MarkersConstants::INDEX_MASK);
        bool wasSet = markers.at(index) != value;
        markers.at(index) = value;
        return wasSet;
    }
    bool IsMarkerSet(Marker mark) const {
        auto value = mark >> static_cast<uint8_t>(MarkersConstants::BIT_LENGTH);
        size_t index =
            mark & static_cast<uint8_t>(MarkersConstants::INDEX_MASK);
        return markers.at(index) == value;
    }
    void ClearMarker(Marker mark) {
        size_t index =
            mark & static_cast<uint8_t>(MarkersConstants::INDEX_MASK);
        markers.at(index) = static_cast<uint8_t>(MarkersConstants::UNDEF_VALUE);
    }

  private:
    std::array<Marker, static_cast<uint8_t>(MarkersConstants::MAX_MARKERS)>
        markers;
};
} // namespace ir

#endif // JIT_AOT_COURSE_MARKER_H_