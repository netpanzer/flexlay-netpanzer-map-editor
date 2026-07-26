#include <QtTest/QtTest>
#include <cstdint>

#include "document.h"

// Document holds the edit rules that used to live inside MapView, where they
// could only be reached through a widget. These are the ones that are easy to
// break by accident: a drag collapsing into one undoable step, redo being
// dropped when a new edit lands, and a stamp clipping at the map edge.

static Map makeMap(int w = 8, int h = 6, uint16_t fill = 0)
{
    Map m;
    m.width  = w;
    m.height = h;
    m.tiles.assign(size_t(w * h), fill);
    return m;
}

class TestDocument : public QObject {
    Q_OBJECT

private slots:
    void initialState()
    {
        Document d;
        d.reset(makeMap());
        QVERIFY(!d.canUndo());
        QVERIFY(!d.canRedo());
        QVERIFY(!d.strokeActive());
        QVERIFY(!d.undo());   // nothing to undo is not an error
        QVERIFY(!d.redo());
    }

    // A drag that crosses many tiles must undo as ONE step, not one per tile.
    void strokeIsOneUndoStep()
    {
        Document d;
        d.reset(makeMap());

        d.beginStroke();
        QVERIFY(d.strokeActive());
        for (int i = 0; i < 5; ++i)
            QVERIFY(d.addToStroke(i, 42));
        QVERIFY(d.commitStroke());
        QVERIFY(!d.strokeActive());

        for (int i = 0; i < 5; ++i)
            QCOMPARE(int(d.map().tiles[size_t(i)]), 42);

        QVERIFY(d.canUndo());
        QVERIFY(d.undo());
        QVERIFY(!d.canUndo());          // one step, not five
        for (int i = 0; i < 5; ++i)
            QCOMPARE(int(d.map().tiles[size_t(i)]), 0);
    }

    // Re-crossing a tile during the same drag must not stack another edit.
    void strokePaintsATileOnce()
    {
        Document d;
        d.reset(makeMap());
        d.beginStroke();
        QVERIFY(d.addToStroke(3, 7));
        QVERIFY(!d.addToStroke(3, 9));   // already painted this stroke
        QVERIFY(d.commitStroke());
        QCOMPARE(int(d.map().tiles[3]), 7);

        d.undo();
        QCOMPARE(int(d.map().tiles[3]), 0);
    }

    // Painting a tile its existing value is not an edit.
    void strokeIgnoresNoOpAndEmptyCommit()
    {
        Document d;
        d.reset(makeMap(8, 6, 5));
        d.beginStroke();
        QVERIFY(!d.addToStroke(0, 5));   // same value
        QVERIFY(!d.commitStroke());      // nothing recorded
        QVERIFY(!d.canUndo());
    }

    void strokeRejectsOutOfRangeIndex()
    {
        Document d;
        d.reset(makeMap(4, 4));
        d.beginStroke();
        QVERIFY(!d.addToStroke(-1, 1));
        QVERIFY(!d.addToStroke(16, 1));  // one past the last tile
        QVERIFY(!d.commitStroke());
    }

    void addToStrokeOutsideAStrokeDoesNothing()
    {
        Document d;
        d.reset(makeMap());
        QVERIFY(!d.addToStroke(0, 3));   // no beginStroke()
        QCOMPARE(int(d.map().tiles[0]), 0);
        QVERIFY(!d.canUndo());
    }

    void undoRedoRoundTrip()
    {
        Document d;
        d.reset(makeMap());
        d.beginStroke();
        d.addToStroke(2, 11);
        d.commitStroke();

        QVERIFY(d.undo());
        QCOMPARE(int(d.map().tiles[2]), 0);
        QVERIFY(d.canRedo());
        QVERIFY(d.redo());
        QCOMPARE(int(d.map().tiles[2]), 11);
        QVERIFY(!d.canRedo());
    }

    // The classic editor rule: editing after an undo discards the redo branch.
    void newEditClearsRedo()
    {
        Document d;
        d.reset(makeMap());
        d.beginStroke(); d.addToStroke(0, 1); d.commitStroke();
        d.undo();
        QVERIFY(d.canRedo());

        d.beginStroke(); d.addToStroke(1, 2); d.commitStroke();
        QVERIFY(!d.canRedo());
        QCOMPARE(int(d.map().tiles[0]), 0);
        QCOMPARE(int(d.map().tiles[1]), 2);
    }

    void resetDiscardsHistory()
    {
        Document d;
        d.reset(makeMap());
        d.beginStroke(); d.addToStroke(0, 1); d.commitStroke();
        QVERIFY(d.canUndo());

        d.reset(makeMap(4, 4));
        QVERIFY(!d.canUndo());
        QVERIFY(!d.canRedo());
        QCOMPARE(d.map().width, 4);
    }

    // An in-progress stroke must not survive loading a different map.
    void resetDropsAnUnfinishedStroke()
    {
        Document d;
        d.reset(makeMap());
        d.beginStroke();
        d.addToStroke(0, 9);
        d.reset(makeMap());
        QVERIFY(!d.strokeActive());
        QVERIFY(!d.canUndo());
        QCOMPARE(int(d.map().tiles[0]), 0);
    }

    void stampAppliesAsOneStep()
    {
        Document d;
        d.reset(makeMap());

        Stamp s;
        s.width = 2; s.height = 2;
        s.tiles = {1, 2, 3, 4};
        QVERIFY(d.applyStamp(s, 1, 1));

        QCOMPARE(int(d.map().tiles[size_t(1 * 8 + 1)]), 1);
        QCOMPARE(int(d.map().tiles[size_t(1 * 8 + 2)]), 2);
        QCOMPARE(int(d.map().tiles[size_t(2 * 8 + 1)]), 3);
        QCOMPARE(int(d.map().tiles[size_t(2 * 8 + 2)]), 4);

        QVERIFY(d.undo());
        QVERIFY(!d.canUndo());          // the whole stamp was one step
        QCOMPARE(int(d.map().tiles[size_t(1 * 8 + 1)]), 0);
    }

    // Stamps may hang off the edge; the parts outside the map are skipped.
    void stampClipsAtTheMapEdge()
    {
        Document d;
        d.reset(makeMap(4, 4));

        Stamp s;
        s.width = 2; s.height = 2;
        s.tiles = {1, 2, 3, 4};
        QVERIFY(d.applyStamp(s, 3, 3));            // only the top-left cell lands
        QCOMPARE(int(d.map().tiles[size_t(3 * 4 + 3)]), 1);
        QCOMPARE((int)d.map().tiles.size(), 16);   // nothing written past the end
    }

    void stampThatChangesNothingIsNotRecorded()
    {
        Document d;
        d.reset(makeMap(4, 4, 6));
        Stamp s;
        s.width = 2; s.height = 2;
        s.tiles = {6, 6, 6, 6};
        QVERIFY(!d.applyStamp(s, 0, 0));
        QVERIFY(!d.canUndo());
    }

    void applyRecordsAndRevertsObjectCommands()
    {
        Document d;
        d.reset(makeMap());

        ObjectRef obj;
        obj.type = "outpost";
        obj.name = "Alpha";
        obj.x = 2; obj.y = 3;
        d.apply(std::make_unique<AddObject>(obj));
        QCOMPARE((int)d.map().objects.size(), 1);

        QVERIFY(d.undo());
        QCOMPARE((int)d.map().objects.size(), 0);
        QVERIFY(d.redo());
        QCOMPARE(d.map().objects[0].name, QString("Alpha"));
    }
};

QTEST_APPLESS_MAIN(TestDocument)
#include "test_document.moc"
