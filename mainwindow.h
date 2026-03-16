#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "canvas.h"
#include "brush.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Canvas* canvas;
    std::vector<Brush> brushes;
    QListWidget *brushList;

    void initBrushes();

    QPoint prev;
private slots:
    void saveImage();
};
#endif // MAINWINDOW_H
