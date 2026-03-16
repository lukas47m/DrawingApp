#include "mainwindow.h"
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>

void MainWindow::initBrushes(){
    brushes.resize(3);
    brushes[1].loadTexture(":/resources/brushes/brush1.png");
    brushes[2].loadTexture(":/resources/brushes/brush2.png");

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    canvas = new Canvas(800,600,this);
    canvas->setChunkSize(17,17);

    setCentralWidget(canvas);

    // LEFT PANEL
    QDockWidget *dock = new QDockWidget("Settings", this);

    QWidget *panel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(panel);

    brushList = new QListWidget(panel);

    brushList->setViewMode(QListView::IconMode);
    brushList->setIconSize(QSize(50,50));
    brushList->setResizeMode(QListView::Adjust);
    brushList->setSpacing(5);

    initBrushes();

    for(int i = 0;i < brushes.size();++i){
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QPixmap::fromImage(brushes[i].texture));
        item->setData(Qt::UserRole, i); // uložíme index brushu
        brushList->addItem(item);
    }

    QPushButton *saveButton = new QPushButton("Save Image");

    layout->addWidget(brushList);
    layout->addWidget(saveButton);



    dock->setWidget(panel);

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);

    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::saveImage()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Image",
        "",
        "PNG Image (*.png);;JPEG Image (*.jpg)"
        );

    if(fileName.isEmpty())
        return;

    canvas->saveImage(fileName);
}

MainWindow::~MainWindow() = default;

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Z) {
            canvas->undo();
        }
        if (event->key() == Qt::Key_Y) {
            canvas->redo();
        }
    }
}


void MainWindow::mousePressEvent(QMouseEvent* event){
    if(event->button() == Qt::LeftButton){
        QPoint p = event->pos() - canvas->pos();
        canvas->draw(p, p ,&brushes[brushList->currentRow()]);
        prev = p;
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent* event){
    if(true){
        QPoint p = event->pos() - canvas->pos();
        canvas->draw(prev, p ,&brushes[brushList->currentRow()]);
        prev = p;
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent* event){
    canvas->saveDiferencies();
    canvas->doCheckpoint();
}