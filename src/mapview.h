#pragma once
#include <QWidget>
#include <QPoint>
#include <QPixmap>
#include <QSet>
#include <vector>
#include "objects.h"
#include "tlsloader.h"

class MapView : public QWidget {
    Q_OBJECT
public:
    static constexpr int TILE_SIZE = 32; // netPanzer tile size in pixels

    explicit MapView(QWidget* parent = nullptr);

    void setMap(const Map& map);
    const Map& map() const { return m_map; }

    // Provide the loaded tileset for actual tile rendering.
    // Pass nullptr to fall back to colour-coded rectangles.
    void setTileset(const Tileset* ts);

    // Zoom
    void setZoom(double z);
    double zoom() const { return m_zoom; }
    void fitToWindow();

    // Editing
    void enableEditing(bool e) { m_editing = e; }
    bool editing() const { return m_editing; }

    void setSelectedTile(int id) { m_selectedTile = id; }
    int  selectedTile() const    { return m_selectedTile; }

    // View
    void setShowGrid(bool show) { m_showGrid = show; update(); }
    bool showGrid() const { return m_showGrid; }

    // Undo/redo
    void undo();
    void redo();
    bool canUndo() const { return !m_undo.empty(); }
    bool canRedo() const { return !m_redo.empty(); }

signals:
    // Emitted on every mouse move over the map (tile coordinates + tile id).
    void tileHovered(int tileX, int tileY, int tileId);
    // Emitted whenever any tile is modified.
    void mapModified();

protected:
    void paintEvent(QPaintEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

private:
    struct TileEdit { int idx; uint16_t oldVal; uint16_t newVal; };

    // Convert a widget-space point to map tile coordinates.
    // Returns false if out of bounds.
    bool widgetToTile(QPoint widgetPos, int& tx, int& ty) const;

    // Paint one tile at the given tile index with the selected tile ID.
    void paintTileAt(int tx, int ty);

    Map      m_map;
    double   m_zoom     = 1.0;
    QPoint   m_pan;
    QPoint   m_lastMouse;
    bool     m_panning  = false;
    bool     m_editing  = false;
    bool     m_showGrid = true;
    int      m_selectedTile = 0;

    const Tileset* m_tileset = nullptr;
    QPixmap        m_atlasPixmap;
    static constexpr int ATLAS_COLS = 64;

    // Tracks which tile indices were edited in the current drag stroke
    // so each tile only gets one undo entry per stroke.
    QSet<int>            m_dragEdited;
    bool                 m_dragging = false;

    std::vector<TileEdit> m_undo;
    std::vector<TileEdit> m_redo;
};
