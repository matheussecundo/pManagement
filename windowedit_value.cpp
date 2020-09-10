#include "windowedit_value.h"
#include "ui_windowedit_value.h"

WindowEdit_Value::WindowEdit_Value(QString value, QWidget *parent) :QDialog(parent),ui(new Ui::WindowEdit_Value){
    ui->setupUi(this);

    ui->valueEdit->setText(value);
}

WindowEdit_Value::~WindowEdit_Value(){
    delete ui;
}


void WindowEdit_Value::on_buttonBox_accepted(){
    newValue = ui->valueEdit->text();
}
