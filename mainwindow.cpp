#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),ui(new Ui::MainWindow){
    ui->setupUi(this);

    ui->adressesValues_TableWidget->setShowGrid(false);
    ui->adressesValues_TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->adressesValues_TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->adressesValues_TableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->adressesValues_TableWidget->verticalHeader()->hide();
    ui->adressesValues_TableWidget->setColumnCount(3);
    QStringList m_TableHeader;
    m_TableHeader << "Address" << "Value" << "Previous";
    ui->adressesValues_TableWidget->setHorizontalHeaderLabels(m_TableHeader);

    ui->selectedAdresses_TableWidget->setShowGrid(false);
    ui->selectedAdresses_TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->selectedAdresses_TableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->selectedAdresses_TableWidget->verticalHeader()->hide();
    ui->selectedAdresses_TableWidget->setColumnCount(3);
    QStringList m_TableHeader2;
    m_TableHeader2 << "Address" << "Type" << "Value";
    ui->selectedAdresses_TableWidget->setHorizontalHeaderLabels(m_TableHeader2);
}

MainWindow::~MainWindow(){
    delete ui;
}

std::vector< pManagement::Address > addressList;

template<typename T>
std::vector< pManagement::Address > cmpValues_perAddress(std::vector< pManagement::Address > &addressList, const T &value, const uint8_t nBytes){
    std::vector< pManagement::Address > newList;
	for(auto &x:addressList){
		if(x.getValue(nBytes) == value){
			newList.push_back(x);
		}
	}
	return newList;
}

uint8_t nBytesType(const int &comboindex){
    switch (comboindex) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 4;
    case 3: return 8;
    case 4: return 4;
    case 5: return 8;
    }
    return 0;
}

template <typename T = uint64_t, typename tvalue>
T TypePunning(tvalue value){
    return *(T*)&value;
}

void MainWindow::on_newSearch_clicked(){
    bool ok;
    int comboindex = ui->typeComboBox->currentIndex();
    uint8_t nBytes = nBytesType(comboindex);

    uint64_t value = (comboindex==3||comboindex==4)? TypePunning(ui->valueLineEdit->text().toDouble(&ok)) : ui->valueLineEdit->text().toULongLong(&ok);

    if(ok){
        ui->adressesValues_TableWidget->setRowCount(0);
        addressList = pH->searchValue(value,nBytes);
        ui->adressesValues_TableWidget->setRowCount(addressList.size());
        char address[10];
        int i = 0;
        for(auto &x:addressList){
            sprintf(address,"%08X",x.address);
            ui->adressesValues_TableWidget->setItem(i,0, new QTableWidgetItem(address));
            ui->adressesValues_TableWidget->setItem(i,1, new QTableWidgetItem(QString::number(x.getValue(nBytes))));
            ui->adressesValues_TableWidget->setItem(i,2, new QTableWidgetItem(QString::number(value)));
            i++;
        }

    }

}

void MainWindow::on_nextValue_clicked(){
    ui->adressesValues_TableWidget->setRowCount(0);
    int comboindex = ui->typeComboBox->currentIndex();
    uint8_t nBytes = nBytesType(comboindex);

    uint64_t value = (comboindex==3||comboindex==4)? TypePunning(ui->valueLineEdit->text().toDouble()) : ui->valueLineEdit->text().toULongLong();
    
    addressList = cmpValues_perAddress(addressList, value, nBytes);

    char address[10];
    int i = 0;
    for(auto &x:addressList){
    	ui->adressesValues_TableWidget->setRowCount(i+1);
        sprintf(address,"%08X",x.address);
        ui->adressesValues_TableWidget->setItem(i,0, new QTableWidgetItem(address));
        ui->adressesValues_TableWidget->setItem(i,1, new QTableWidgetItem(QString::number(value)));
        ui->adressesValues_TableWidget->setItem(i,2, new QTableWidgetItem(QString::number(value)));
        i++;
    }
}

void MainWindow::on_actionOpen_Process_triggered(){
    OpenProcessWindow op_window;
    op_window.setModal(true);
    op_window.exec();
    if(op_window.proc_loaded){
        if(pH) delete pH;
        pH = new pManagement::pHandle(op_window.proc_loaded->getPID());
    }
}

void MainWindow::on_adressesValues_TableWidget_cellDoubleClicked(int row, int column){
    QString address = ui->adressesValues_TableWidget->selectedItems()[0]->text();
    int comboindex = ui->typeComboBox->currentIndex();
    uint8_t nBytes = nBytesType(comboindex);
    uint8_t Buffer[8] = {0};
    pH->ReadMemory(address.toUInt(0,16),Buffer,nBytes);

    int nrow = ui->selectedAdresses_TableWidget->rowCount();
    ui->selectedAdresses_TableWidget->setRowCount(nrow+1);
    ui->selectedAdresses_TableWidget->setItem(nrow,0, new QTableWidgetItem(address));
    ui->selectedAdresses_TableWidget->setItem(nrow,1, new QTableWidgetItem(ui->typeComboBox->currentText()));
    ui->selectedAdresses_TableWidget->setItem(nrow,2, new QTableWidgetItem(QString::number(*(uint64_t*)Buffer)));
}



void MainWindow::on_selectedAdresses_TableWidget_cellDoubleClicked(int row, int column){
    QTableWidgetItem *item = ui->selectedAdresses_TableWidget->item(row,column);
    switch(column){
    case 0: {
        break;
    }
    case 1:{
        break;
    }
    case 2:{
        WindowEdit_Value window(item->text());
        window.exec();
        if(window.newValue!=""){
            uint32_t address = ui->selectedAdresses_TableWidget->item(item->row(),0)->text().toUInt(0,16);
            int comboindex = ui->typeComboBox->currentIndex();
            uint8_t nBytes = nBytesType(comboindex);

            uint64_t newValue = (comboindex==3||comboindex==4)? TypePunning(window.newValue.toDouble()) : window.newValue.toULongLong();

            uint8_t Buffer[8] = {0};
            *(uint64_t*)Buffer = newValue;
            pH->WriteMemory(address,Buffer,nBytes);
        }
        break;
    }
    default: qDebug() << row << " " << column;
    }
}
