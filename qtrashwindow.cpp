#include "qtrashwindow.h"
#include "ui_qtrashwindow.h"

#include <QMenu>

#include "qtrashmodel.h"

QTrashWindow::QTrashWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QTrashWindow)
{
    ui->setupUi(this);

    m_model = new QTrashModel(this);
    ui->treeView->setModel(m_model);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));

    connect(ui->actionRestore, SIGNAL(triggered()), SLOT(restore()));
    connect(ui->actionRemove, SIGNAL(triggered()), SLOT(remove()));
    connect(ui->actionEmptyTrash, SIGNAL(triggered()), SLOT(emptyTrash()));
}

QTrashWindow::~QTrashWindow()
{
    delete ui;
}

void QTrashWindow::restore()
{
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();

    if (indexes.isEmpty())
        return;

    m_model->restore(indexes.first());
}

void QTrashWindow::remove()
{
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();

    if (indexes.isEmpty())
        return;

    m_model->remove(indexes.first());
}

void QTrashWindow::emptyTrash()
{
    m_model->clearTrash();
}

void QTrashWindow::showContextMenu(QPoint p)
{
    QMenu menu;
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();

    if (!indexes.isEmpty()) {
        menu.addAction(ui->actionRestore);
        menu.addSeparator();
        menu.addAction(ui->actionRemove);
        menu.addSeparator();
    }

    menu.addAction(ui->actionEmptyTrash);
    menu.exec(ui->treeView->viewport()->mapToGlobal(p));
}
