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
  QGroupBox *gbTrick = findChild<QGroupBox *>("gbTrick");

  QHBoxLayout *gbPlayer1Layout = new QHBoxLayout();
  QHBoxLayout *gbPlayer2Layout = new QHBoxLayout();
  QHBoxLayout *gbPlayer3Layout = new QHBoxLayout();
  QHBoxLayout *gbSkatLayout = new QHBoxLayout();
  QHBoxLayout *gbTrickLayout = new QHBoxLayout();

  gbPlayer1->setLayout(gbPlayer1Layout);
  gbPlayer2->setLayout(gbPlayer2Layout);
  gbPlayer3->setLayout(gbPlayer3Layout);
  gbSkat->setLayout(gbSkatLayout);
  gbTrick->setLayout(gbTrickLayout);

  for (const Card &card : game_.player_1.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    gbPlayer1Layout->addWidget(cardButton);

    // Optional: Connect button to an event (e.g., play card)
    QObject::connect(
        cardButton, &QPushButton::clicked, this,
        [this, cardButton, gbPlayer1Layout, gbTrickLayout, card]() {
          if (game_.playerList_[0]->id() == game_.player_1.id()) {
            // ✅ First, check if the move is valid before doing anything
            if (!game_.isCardValid(card)) {
              qDebug() << "Move rejected: Invalid card choice.";
              return;  // 🚨 Exit early if the move is not valid
            }

            // ✅ Only remove the card if it's valid
            gbPlayer1Layout->removeWidget(cardButton);
            cardButton->setParent(nullptr);  // Remove parent to avoid conflicts

            // ✅ Play the card
            game_.playCard(card);

            // ✅ Move card to trick only after it has been successfully played
            gbTrickLayout->addWidget(cardButton);
          }
        });
  }

  for (const Card &card : game_.skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    gbSkatLayout->addWidget(cardButton);
  }

  for (const Card &card : game_.player_2.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    gbPlayer2Layout->addWidget(cardButton);
    QObject::connect(
        cardButton, &QPushButton::clicked, this,
        [this, cardButton, gbPlayer2Layout, gbTrickLayout, card]() {
          if (game_.playerList_[0]->id() == game_.player_2.id()) {
            // ✅ First, check if the move is valid before doing anything
            if (!game_.isCardValid(card)) {
              qDebug() << "Move rejected: Invalid card choice.";
              return;  // 🚨 Exit early if the move is not valid
            }

            // ✅ Only remove the card if it's valid
            gbPlayer2Layout->removeWidget(cardButton);
            cardButton->setParent(nullptr);  // Remove parent to avoid conflicts

            // ✅ Play the card
            game_.playCard(card);

            // ✅ Move card to trick only after it has been successfully played
            gbTrickLayout->addWidget(cardButton);
          }
        });
  }

  for (const Card &card : game_.player_3.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // UTF-8 compatible
    gbPlayer3Layout->addWidget(cardButton);
    QObject::connect(
        cardButton, &QPushButton::clicked, this,
        [this, cardButton, gbPlayer3Layout, gbTrickLayout, card]() {
          if (game_.playerList_[0]->id() == game_.player_3.id()) {
            // ✅ First, check if the move is valid before doing anything
            if (!game_.isCardValid(card)) {
              qDebug() << "Move rejected: Invalid card choice.";
              return;  // 🚨 Exit early if the move is not valid
            }

            // ✅ Only remove the card if it's valid
            gbPlayer3Layout->removeWidget(cardButton);
            cardButton->setParent(nullptr);  // Remove parent to avoid conflicts

            // ✅ Play the card
            game_.playCard(card);

            // ✅ Move card to trick only after it has been successfully played
            gbTrickLayout->addWidget(cardButton);
          }
        });
  }

  QObject::connect(&game_, &Game::clearTrickLayout, this,
                   &Table::onClearTrickLayout);
}

void Table::onClearTrickLayout() {
  qDebug() << "called onClearTrickLayout";

  QLayout *layout = findChild<QGroupBox *>("gbTrick")->layout();
  if (!layout) return;

  while (QLayoutItem *item = layout->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
}

Table::~Table() { delete ui; }
