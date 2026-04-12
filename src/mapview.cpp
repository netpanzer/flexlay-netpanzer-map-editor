#include "mapview.h"
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <algorithm>

MapView::MapView(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ---------------------------------------------------------------------------
// Map / tileset

void MapView::setMap(const Map& map)
{
    m_map = map;
    m_pan = QPoint(0, 0);
    m_zoom = 1.0;
    m_undo.clear();
    m_redo.clear();
    m_dragEdited.clear();
    m_dragging = false;
    update();
}

void MapView::setTileset(const Tileset* ts)
{
    m_tileset = ts;
    m_atlasPixmap = QPixmap(); // invalidate cache
    update();
}

void MapView::setZoom(double z)
{
    m_zoom = std::clamp(z, 0.05, 16.0);
    update();
}

void MapView::fitToWindow()
{
    if (m_map.width <= 0 || m_map.height <= 0) return;
    const double ws = double(width())  / double(m_map.width  * TILE_SIZE);
    const double hs = double(height()) / double(m_map.height * TILE_SIZE);
    m_zoom = std::min(ws, hs);
    m_pan  = QPoint((width()  - int(m_map.width  * TILE_SIZE * m_zoom)) / 2,
                    (height() - int(m_map.height * TILE_SIZE * m_zoom)) / 2);
    update();
}

// ---------------------------------------------------------------------------
// Coordinate helpers

bool MapView::widgetToTile(QPoint wpos, int& tx, int& ty) const
{
    if (m_map.width <= 0 || m_map.height <= 0) return false;
    const QPointF mp = (QPointF(wpos) - QPointF(m_pan)) / m_zoom;
    tx = int(mp.x()) / TILE_SIZE;
    ty = int(mp.y()) / TILE_SIZE;
    return tx >= 0 && ty >= 0 && tx < m_map.width && ty < m_map.height;
}

// ---------------------------------------------------------------------------
// Painting

void MapView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(40, 40, 40));

    if (!m_map.isValid()) {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "No map loaded.\nUse File → Open to load a .npm file.");
        return;
    }

    // Build atlas pixmap on first use (requires display context)
    if (m_tileset && m_tileset->isValid() && m_atlasPixmap.isNull())
        m_atlasPixmap = QPixmap::fromImage(m_tileset->atlas(ATLAS_COLS));

    p.save();
    p.translate(m_pan);
    p.scale(m_zoom, m_zoom);

    const QRectF mapRect(0, 0,
                         m_map.width  * TILE_SIZE,
                         m_map.height * TILE_SIZE);

    // Determine which tile columns/rows are visible
    const QRectF visible = p.transform().inverted().mapRect(QRectF(rect()));
    const int x0 = std::max(0, int(visible.left()  / TILE_SIZE));
    const int y0 = std::max(0, int(visible.top()   / TILE_SIZE));
    const int x1 = std::min(m_map.width  - 1, int(visible.right()  / TILE_SIZE));
    const int y1 = std::min(m_map.height - 1, int(visible.bottom() / TILE_SIZE));

    if (!m_atlasPixmap.isNull()) {
        // Tile rendering: draw from atlas
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                const int id = m_map.tiles[size_t(y * m_map.width + x)];
                const QRect src = m_tileset->atlasRect(id, ATLAS_COLS);
                const QRectF dst(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                p.drawPixmap(dst, m_atlasPixmap, QRectF(src));
            }
        }
    } else {
        // Fallback: colour-coded rectangles
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                const int v = m_map.tiles[size_t(y * m_map.width + x)];
                p.fillRect(QRectF(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE),
                           QColor::fromHsv((v * 37) % 360, 180, 200));
            }
        }
    }

    // Grid
    if (m_showGrid && m_zoom > 0.2) {
        const QColor gridCol = m_atlasPixmap.isNull()
                               ? QColor(0, 0, 0, 60) : QColor(0, 0, 0, 80);
        p.setPen(QPen(gridCol, 0)); // cosmetic pen (1 screen pixel)
        for (int y = y0; y <= y1 + 1; ++y)
            p.drawLine(QPointF(x0 * TILE_SIZE, y * TILE_SIZE),
                       QPointF((x1 + 1) * TILE_SIZE, y * TILE_SIZE));
        for (int x = x0; x <= x1 + 1; ++x)
            p.drawLine(QPointF(x * TILE_SIZE, y0 * TILE_SIZE),
                       QPointF(x * TILE_SIZE, (y1 + 1) * TILE_SIZE));
    }

    // Objects — coordinates are in tile units; render as labelled circles
    p.setRenderHint(QPainter::Antialiasing);
    for (const auto& obj : m_map.objects) {
        // Convert tile coords to pixel coords (centre of tile)
        const QPointF centre((obj.x + 0.5) * TILE_SIZE, (obj.y + 0.5) * TILE_SIZE);
        const double r = TILE_SIZE * 0.45;

        QColor fill = (obj.type == "outpost") ? QColor(220, 60, 60, 200)
                                              : QColor(60, 100, 220, 200);
        p.setBrush(fill);
        p.setPen(QPen(Qt::white, 0.5));
        p.drawEllipse(centre, r, r);

        if (m_zoom >= 0.4) {
            p.setPen(Qt::white);
            QFont f = p.font();
            f.setPixelSize(std::max(6, int(10 / m_zoom)));
            p.setFont(f);
            p.drawText(QRectF(centre.x() - r, centre.y() - r, r * 2, r * 2),
                       Qt::AlignCenter,
                       obj.type == "outpost" ? "O" : "S");
        }
    }

    // Map border
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(200, 200, 200), 0));
    p.drawRect(mapRect);

    p.restore();
}

// ---------------------------------------------------------------------------
// Wheel zoom (zoom toward the mouse pointer)

void MapView::wheelEvent(QWheelEvent* ev)
{
    const double factor = (ev->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    const QPointF pos = ev->position();
    // Zoom around cursor
    const QPointF mapPt = (pos - QPointF(m_pan)) / m_zoom;
    m_zoom = std::clamp(m_zoom * factor, 0.05, 16.0);
    m_pan = QPoint(int(pos.x() - mapPt.x() * m_zoom),
                   int(pos.y() - mapPt.y() * m_zoom));
    update();
    ev->accept();
}

// ---------------------------------------------------------------------------
// Tile painting

void MapView::paintTileAt(int tx, int ty)
{
    const int idx = ty * m_map.width + tx;
    if (m_dragEdited.contains(idx)) return; // already painted in this stroke

    const uint16_t oldVal = m_map.tiles[size_t(idx)];
    const uint16_t newVal = uint16_t(m_selectedTile);
    if (oldVal == newVal) return;

    m_map.tiles[size_t(idx)] = newVal;
    m_dragEdited.insert(idx);
    m_undo.push_back({idx, oldVal, newVal});
    m_redo.clear();
    update();
    emit mapModified();
}

// ---------------------------------------------------------------------------
// Mouse events

void MapView::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::MiddleButton) {
        m_panning   = true;
        m_lastMouse = ev->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (m_editing && ev->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragEdited.clear();
        int tx, ty;
        if (widgetToTile(ev->pos(), tx, ty))
            paintTileAt(tx, ty);
    }
}

void MapView::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_panning) {
        m_pan += ev->pos() - m_lastMouse;
        m_lastMouse = ev->pos();
        update();
        return;
    }

    if (m_editing && m_dragging && (ev->buttons() & Qt::LeftButton)) {
        int tx, ty;
        if (widgetToTile(ev->pos(), tx, ty))
            paintTileAt(tx, ty);
    }

    // Emit hover info for status bar
    int tx, ty;
    if (widgetToTile(ev->pos(), tx, ty)) {
        const int id = m_map.tiles[size_t(ty * m_map.width + tx)];
        emit tileHovered(tx, ty, id);
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::MiddleButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        return;
    }
    if (ev->button() == Qt::LeftButton) {
        m_dragging = false;
        m_dragEdited.clear();
    }
}

void MapView::leaveEvent(QEvent*)
{
    m_dragging = false;
    m_dragEdited.clear();
}

// ---------------------------------------------------------------------------
// Undo / redo

void MapView::undo()
{
    if (m_undo.empty()) return;
    TileEdit e = m_undo.back();
    m_undo.pop_back();
    m_map.tiles[size_t(e.idx)] = e.oldVal;
    m_redo.push_back(e);
    update();
    emit mapModified();
}

void MapView::redo()
{
    if (m_redo.empty()) return;
    TileEdit e = m_redo.back();
    m_redo.pop_back();
    m_map.tiles[size_t(e.idx)] = e.newVal;
    m_undo.push_back(e);
    update();
    emit mapModified();
}
