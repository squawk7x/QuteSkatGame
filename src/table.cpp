#include "table.h"

#include "./ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Table) {
  ui->setupUi(this);

  game_.initGame();

  QGroupBox *gbPlayer1 = findChild<QGroupBox *>("gbPlayer1");
  QGroupBox *gbPlayer2 = findChild<QGroupBox *>("gbPlayer2");
  QGroupBox *gbPlayer3 = findChild<QGroupBox *>("gbPlayer3");
  QGroupBox *gbSkat = findChild<QGroupBox *>("gbSkat");

  QHBoxLayout *gbPlayer1Layout = new QHBoxLayout();
  QHBoxLayout *gbPlayer2Layout = new QHBoxLayout();
  QHBoxLayout *gbPlayer3Layout = new QHBoxLayout();
  QHBoxLayout *gbSkatLayout = new QHBoxLayout();

  gbPlayer1->setLayout(gbPlayer1Layout);
  gbPlayer2->setLayout(gbPlayer2Layout);
  gbPlayer3->setLayout(gbPlayer3Layout);
  gbSkat->setLayout(gbSkatLayout);

  for (const Card &card : game_.player_1.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    // cardButton->setStyleSheet(
    //     "font-size: 20px; padding: 5px; border: 1px solid black;");

    gbPlayer1Layout->addWidget(cardButton);

    // Optional: Connect button to an event (e.g., play card)
    // connect(cardButton, &QPushButton::clicked, this, [this, card]() {
    //   playCard(card);
    // });
  }

  for (const Card &card : game_.skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    // cardButton->setStyleSheet(
    //     "font-size: 20px; padding: 5px; border: 1px solid black;");

    gbSkatLayout->addWidget(cardButton);
  }

  for (const Card &card : game_.player_2.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    // cardButton->setStyleSheet(
    //     "font-size: 20px; padding: 5px; border: 1px solid black;");

    gbPlayer2Layout->addWidget(cardButton);
  }

  for (const Card &card : game_.player_3.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    // cardButton->setStyleSheet(
    //     "font-size: 20px; padding: 5px; border: 1px solid black;");

    gbPlayer3Layout->addWidget(cardButton);
  }
}

Table::~Table() { delete ui; }
