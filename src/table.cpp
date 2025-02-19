#include "table.h"

#include "./ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Table) {
  ui->setupUi(this);

  game_.initGame();
}

Table::~Table() { delete ui; }
