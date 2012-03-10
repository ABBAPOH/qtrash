#ifndef QTRASHWINDOW_H
#define QTRASHWINDOW_H

#include <QWidget>

class QTrashModel;

namespace Ui {
    class QTrashWindow;
}

class QTrashWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QTrashWindow(QWidget *parent = 0);
    ~QTrashWindow();

private slots:
    void restore();
    void remove();
    void emptyTrash();
    void showContextMenu(QPoint p);

private:
    Ui::QTrashWindow *ui;

    QTrashModel *m_model;
};

#endif // QTRASHWINDOW_H
