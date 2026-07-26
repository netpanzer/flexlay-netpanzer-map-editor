#pragma once
#include <QAbstractSlider>
#include <QKeyEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>

// The tile and stamp panels are the scrolled child of a QScrollArea, so they
// receive Home/End/PageUp/PageDown themselves and the scroll area never sees
// them. Forward those keys to its vertical scrollbar.
//
// Returns true when the key was consumed; callers should then return without
// chaining to the base class.
inline bool forwardScrollKeys(QKeyEvent* ev, QWidget* panel)
{
    QWidget* const grandparent =
        panel->parentWidget() ? panel->parentWidget()->parentWidget() : nullptr;
    auto* const area = qobject_cast<QScrollArea*>(grandparent);
    if (!area) return false;

    QScrollBar* const bar = area->verticalScrollBar();
    switch (ev->key()) {
    case Qt::Key_Home:     bar->triggerAction(QAbstractSlider::SliderToMinimum);   return true;
    case Qt::Key_End:      bar->triggerAction(QAbstractSlider::SliderToMaximum);   return true;
    case Qt::Key_PageUp:   bar->triggerAction(QAbstractSlider::SliderPageStepSub); return true;
    case Qt::Key_PageDown: bar->triggerAction(QAbstractSlider::SliderPageStepAdd); return true;
    default:               return false;
    }
}
