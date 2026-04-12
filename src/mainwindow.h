#pragma once
#include <QMainWindow>
#include <QString>
#include "maploader.h"
#include "tlsloader.h"

class MapView;
class TilePanel;
class QLabel;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onOpen();
    void onSave();
    void onSaveAs();
    void onNewMap();
    void onLoadTileset();
    void onUndo();
    void onRedo();
    void onToggleEdit(bool checked);
    void onToggleGrid(bool checked);
    void onFitToWindow();
    void onZoomIn();
    void onZoomOut();
    void onTileHovered(int tx, int ty, int tileId);
    void onMapModified();

private:
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setCurrentFile(const QString& path);
    void updateTitle();
    void applyTileset();
    // Try to find the tileset file from the map's tileSetName alongside the map.
    QString findTileset(const QString& mapPath, const QString& tileSetName) const;

    MapView*   m_view     = nullptr;
    TilePanel* m_tilePanel = nullptr;
    QLabel*    m_statusTile = nullptr;
    QLabel*    m_statusZoom = nullptr;

    QAction*   m_undoAct  = nullptr;
    QAction*   m_redoAct  = nullptr;
    QAction*   m_saveAct  = nullptr;
    QAction*   m_editToggle = nullptr;

    QString    m_currentFile;
    bool       m_modified = false;

    Tileset    m_tileset;
};
