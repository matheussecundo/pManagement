#include "openprocesswindow.h"
#include "ui_openprocesswindow.h"


OpenProcessWindow::OpenProcessWindow(QWidget *parent) :QDialog(parent),ui(new Ui::OpenProcessWindow){
    ui->setupUi(this);

    ui->tableProcessWidget->setShowGrid(false);
    ui->tableProcessWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableProcessWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableProcessWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableProcessWidget->setColumnCount(2);
    QStringList m_TableHeader;
    m_TableHeader << "PID" << "ProcessName";
    ui->tableProcessWidget->setHorizontalHeaderLabels(m_TableHeader);

    plist = new pView::ProcessList;
    auto parray = plist->getProcesses();
    ui->tableProcessWidget->setRowCount(parray.size());
    char text[12];
    int i = 0;
    for(auto p:parray){
        sprintf(text,"0x%08X",p.getPID());
        ui->tableProcessWidget->setItem(i,0, new QTableWidgetItem(text));
        ui->tableProcessWidget->setItem(i,1, new QTableWidgetItem(p.getProcessName().c_str()));
        i++;
    }

}

OpenProcessWindow::~OpenProcessWindow(){
    delete plist;
    plist = 0;
    delete ui;
}

void OpenProcessWindow::on_buttonBox_accepted(){
    uint32_t PID = 0;
    std::string cpid( ui->tableProcessWidget->selectedItems()[0]->text().toStdString());
    sscanf(cpid.c_str(), "%x", &PID);
    proc_loaded = new pView::iProcess((*plist)[PID]);
}
