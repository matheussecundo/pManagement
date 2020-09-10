#ifndef WINDOWEDIT_VALUE_H
#define WINDOWEDIT_VALUE_H

#include <QDialog>

namespace Ui {
class WindowEdit_Value;
}

class WindowEdit_Value : public QDialog{
    Q_OBJECT

public:
    explicit WindowEdit_Value(QString value,QWidget *parent = nullptr);
    ~WindowEdit_Value();

    QString newValue;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::WindowEdit_Value *ui;
};

#endif // WINDOWEDIT_VALUE_H
