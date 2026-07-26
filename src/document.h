#pragma once
#include <QSet>
#include <memory>
#include <vector>
#include "commands.h"
#include "objects.h"
#include "stamp.h"

// The edited map and its undo history.
//
// Deliberately free of widget code: every edit rule lives here, so the
// semantics that are easy to get wrong — a drag-paint collapsing into one
// undoable step, redo being dropped once a new edit lands — can be tested
// without constructing a MapView or a QApplication. MapView owns one of these
// and is responsible for repainting and signalling after it mutates.
class Document {
public:
    // Load a map and discard any history belonging to the previous one.
    void reset(const Map& map);

    const Map& map() const { return m_map; }
    Map&       map()       { return m_map; }

    // Apply a command and record it. For edits the caller has not made yet.
    void apply(std::unique_ptr<Command> cmd);
    // Record a command whose effect the caller already applied to map().
    void record(std::unique_ptr<Command> cmd);

    bool canUndo() const { return !m_undo.empty(); }
    bool canRedo() const { return !m_redo.empty(); }
    bool undo();   // false when there was nothing to undo
    bool redo();

    // --- Stroke batching -----------------------------------------------------
    // A drag-paint is many small edits that must undo as one. beginStroke()
    // starts collecting; addToStroke() paints a tile at most once per stroke;
    // commitStroke() records the batch and reports whether anything changed.
    void beginStroke();
    bool addToStroke(int tileIndex, uint16_t value);
    bool commitStroke();
    bool strokeActive() const { return m_stroke != nullptr; }

    // Paint a stamp with its top-left at (tx, ty), clipped to the map.
    // Recorded as a single undoable step. Returns false when nothing changed.
    bool applyStamp(const Stamp& stamp, int tx, int ty);

private:
    Map m_map;
    std::vector<std::unique_ptr<Command>> m_undo;
    std::vector<std::unique_ptr<Command>> m_redo;

    std::unique_ptr<TileBatch> m_stroke;
    QSet<int>                  m_strokeTiles;  // indices already painted this stroke
};
