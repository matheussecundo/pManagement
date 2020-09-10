#ifndef OPENPROCESSWINDOW_H
#define OPENPROCESSWINDOW_H

#include <QDialog>
#include "pmanagement.h"

#include <QDebug>

namespace Ui {
class OpenProcessWindow;
}

class OpenProcessWindow : public QDialog{
    Q_OBJECT

public:
    explicit OpenProcessWindow(QWidget *parent = nullptr);
    ~OpenProcessWindow();

    pView::ProcessList *plist = 0;
    pView::iProcess *proc_loaded = 0;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::OpenProcessWindow *ui;
};

#endif // OPENPROCESSWINDOW_H
