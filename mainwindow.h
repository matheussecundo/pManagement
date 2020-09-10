#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "openprocesswindow.h"
#include "windowedit_value.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    pManagement::pHandle *pH = 0;

private slots:

    void on_newSearch_clicked();

    void on_nextValue_clicked();

    void on_actionOpen_Process_triggered();

    void on_adressesValues_TableWidget_cellDoubleClicked(int row, int column);

    void on_selectedAdresses_TableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
