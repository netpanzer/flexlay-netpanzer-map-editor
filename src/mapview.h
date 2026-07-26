#pragma once
#include <QWidget>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QPixmap>
#include <QSet>
#include <vector>
#include <memory>
#include <optional>
#include "objects.h"
#include "tlsloader.h"
#include "commands.h"
#include "document.h"
#include "stamp.h"

enum class Tool {
    TilePaint,       // left-drag paints selected tile
    EllipsePaint,    // drag to paint selected tile along ellipse outline
    RectOutline,     // drag to paint selected tile along rectangle outline
    TilePick,        // left-click picks tile under cursor, switches to TilePaint
    RectSelect,      // drag to select a rectangular tile region
    RectFill,        // drag to fill a rectangular tile region
    StampPaint,      // click to place the current stamp
    PlaceOutpost,    // left-click places an outpost
    PlaceSpawnpoint, // left-click places a spawn point
    SelectObject     // left-click selects/drags objects; Del removes
};

class MapView : public QWidget {
    Q_OBJECT
public:
    static constexpr int TILE_SIZE = 32;

    explicit MapView(QWidget* parent = nullptr);

    void setMap(const Map& map);
    const Map& map() const { return m_doc.map(); }
    Map&       map()       { return m_doc.map(); }

    void setTileset(const Tileset* ts);
    const Tileset* tileset() const { return m_tileset; }

    // Zoom / pan
    void setZoom(double z);
    double zoom() const { return m_zoom; }
    void   fitToWindow();

    // Tool
    void setTool(Tool t);
    Tool tool() const { return m_tool; }

    // Tile painting
    void setSelectedTile(int id) { m_selectedTile = id; }

    // Rect selection
    QRect selection() const { return m_selection; }
    bool  hasSelection() const { return !m_selection.isNull(); }
    Stamp captureSelection() const;

    // Stamp painting
    void setCurrentStamp(const Stamp* stamp);

    // View options
    void setShowGrid(bool show) { m_showGrid = show; update(); }

    // Object access
    void deleteSelectedObject();

    // Apply a command to the map (apply + push to undo stack).
    // For use by external code (e.g. MainWindow rename dialog).
    void applyCommand(std::unique_ptr<Command> cmd);

    // Pan so that the given tile coordinate is centred in the viewport.
    void panToTile(QPointF tilePt);

    // Undo / redo
    void undo();
    void redo();
    bool canUndo() const { return m_doc.canUndo(); }
    bool canRedo() const { return m_doc.canRedo(); }

signals:
    void tileHovered(int tileX, int tileY, int tileId);
    void tilePicked(int tileId);        // emitted when TilePick tool clicks a tile
    void toolChanged(Tool t);           // emitted when tool changes internally (e.g. pick→paint)
    void selectionChanged(QRect sel);   // emitted when rect selection changes
    void mapModified();
    void objectSelectionChanged(int idx); // -1 = none
    void objectActivated(int idx);        // double-click on an object
    void viewportChanged(QRectF tileRect);
    void stampDeselected();               // right-click or Escape in stamp mode

protected:
    void paintEvent(QPaintEvent*)         override;
    void wheelEvent(QWheelEvent*)         override;
    void mousePressEvent(QMouseEvent*)    override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*)     override;
    void mouseReleaseEvent(QMouseEvent*)  override;
    void leaveEvent(QEvent*)              override;
    void keyPressEvent(QKeyEvent*)        override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void resizeEvent(QResizeEvent*)       override;

private:
    // A rubber-band drag in tile coordinates. The shape tools differ only in
    // which tiles they derive from the two endpoints, so they share this.
    struct Drag {
        QPoint start;
        QPoint end;
        bool   active = false;

        void begin(QPoint tile) { start = end = tile; active = true; }
    };

    // Coordinate mapping. Returns nullopt when the position is outside the map,
    // which is the only failure the callers care about.
    std::optional<QPoint> tileAt(QPoint widgetPos) const;
    QPointF widgetToMapPx(QPoint widgetPos) const;

    // Object hit test (returns index or -1)
    int objectAt(QPoint widgetPos) const;

    // Tile paint helpers
    void startStroke();
    void addToStroke(int tx, int ty);
    void commitStroke();

    // Paint a whole tile set as one undoable stroke (used by the shape tools).
    void strokeTiles(const std::vector<QPoint>& tiles);

    // Command stack (unified for tiles and objects)
    // pushCommand: command already applied to m_map (tile batches)
    void pushCommand(std::unique_ptr<Command> cmd);

    void emitViewportChanged();

    // Map + undo history
    Document       m_doc;

    // Rendering
    double         m_zoom     = 1.0;
    QPoint         m_pan;
    bool           m_showGrid = true;

    const Tileset* m_tileset  = nullptr;
    QPixmap        m_atlasPixmap;
    static constexpr int ATLAS_COLS = 64;

    // Tool state
    Tool m_tool        = Tool::TilePaint;
    int  m_selectedTile = 0;
    int  m_selectedObj  = -1;

    // Rect selection state
    QRect  m_selection;          // in tile coords, null if none
    bool   m_selecting  = false;
    QPoint m_selectStart;        // tile coord where drag began

    // Rect fill state
    bool   m_rectFilling  = false;
    QPoint m_rectFillStart;
    QRect  m_rectFillPreview;

    // Stamp paint state
    const Stamp* m_currentStamp    = nullptr;
    QPoint       m_stampHoverTile  = QPoint(-1, -1);
    bool         m_stampPainting   = false;

    // Outpost placement hover (tile coords, (-1,-1) = none)
    QPoint m_outpostHoverTile = QPoint(-1, -1);

    // Shape paint state
    Drag m_ellipse;
    Drag m_rectOutline;

    static std::vector<QPoint> computeEllipseTiles(QPoint a, QPoint b, int mapW, int mapH);
    static std::vector<QPoint> computeRectOutlineTiles(QPoint a, QPoint b, int mapW, int mapH);

    // Tiles the active shape tool would paint, or empty when none is dragging.
    std::vector<QPoint> activeShapeTiles() const;

    // paintEvent stages, in draw order. All draw in map-pixel coordinates, with
    // the pan/zoom transform already applied by the caller. `visible` is the
    // inclusive tile range currently on screen.
    void drawTiles(QPainter& p, QRect visible) const;
    void drawStampGhost(QPainter& p) const;
    void drawRectFillPreview(QPainter& p) const;
    void drawSelection(QPainter& p) const;
    void drawGrid(QPainter& p, QRect visible) const;
    void drawObjects(QPainter& p) const;
    void drawShapePreview(QPainter& p) const;
    void drawCapturePads(QPainter& p) const;

    // Pan state (middle button)
    bool   m_panning   = false;
    QPoint m_lastMouse;

    void applyStamp(int tx, int ty);

    // Object drag state
    bool m_draggingObj  = false;
    int  m_objDragOrigX = 0;
    int  m_objDragOrigY = 0;

    // Command stacks
    std::vector<std::unique_ptr<Command>> m_undo;
    std::vector<std::unique_ptr<Command>> m_redo;
};
