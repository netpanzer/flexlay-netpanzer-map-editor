#include "document.h"

void Document::reset(const Map& map)
{
    m_map = map;
    m_undo.clear();
    m_redo.clear();
    m_stroke.reset();
    m_strokeTiles.clear();
}

void Document::apply(std::unique_ptr<Command> cmd)
{
    cmd->apply(m_map);
    record(std::move(cmd));
}

void Document::record(std::unique_ptr<Command> cmd)
{
    m_undo.push_back(std::move(cmd));
    // Anything previously undone is unreachable once a new edit lands.
    m_redo.clear();
}

bool Document::undo()
{
    if (m_undo.empty()) return false;
    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();
    cmd->revert(m_map);
    m_redo.push_back(std::move(cmd));
    return true;
}

bool Document::redo()
{
    if (m_redo.empty()) return false;
    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();
    cmd->apply(m_map);
    m_undo.push_back(std::move(cmd));
    return true;
}

void Document::beginStroke()
{
    m_stroke = std::make_unique<TileBatch>();
    m_strokeTiles.clear();
}

bool Document::addToStroke(int tileIndex, uint16_t value)
{
    if (!m_stroke) return false;
    if (tileIndex < 0 || size_t(tileIndex) >= m_map.tiles.size()) return false;
    // Re-crossing a tile mid-drag must not stack another edit on it.
    if (m_strokeTiles.contains(tileIndex)) return false;
    m_strokeTiles.insert(tileIndex);

    const uint16_t previous = m_map.tiles[size_t(tileIndex)];
    if (previous == value) return false;

    m_map.tiles[size_t(tileIndex)] = value;
    m_stroke->edits.push_back({tileIndex, previous, value});
    return true;
}

bool Document::commitStroke()
{
    const bool changed = m_stroke && !m_stroke->empty();
    if (changed)
        record(std::move(m_stroke));
    m_stroke.reset();
    m_strokeTiles.clear();
    return changed;
}

bool Document::applyStamp(const Stamp& stamp, int tx, int ty)
{
    if (!m_map.isValid()) return false;

    auto batch = std::make_unique<TileBatch>();
    for (int row = 0; row < stamp.height; ++row) {
        for (int col = 0; col < stamp.width; ++col) {
            const int mtx = tx + col;
            const int mty = ty + row;
            if (mtx < 0 || mty < 0 || mtx >= m_map.width || mty >= m_map.height)
                continue;   // stamps may hang off the edge of the map
            const int idx = mty * m_map.width + mtx;
            const uint16_t previous = m_map.tiles[size_t(idx)];
            const uint16_t value    = stamp.tiles[size_t(row * stamp.width + col)];
            if (previous == value) continue;
            m_map.tiles[size_t(idx)] = value;
            batch->edits.push_back({idx, previous, value});
        }
    }
    if (batch->empty()) return false;
    record(std::move(batch));
    return true;
}
