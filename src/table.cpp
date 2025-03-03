#include "table.h"

#include <qthread.h>

#include <QTimer>

#include "ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch), game_(new Game(this)) {
  ui->setupUi(this);

  // connect pushbuttons
  {
    QObject::connect(ui->pbKaro, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♦";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbHerz, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♥";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbPik, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♠";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbKreuz, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Suit;
      game_->trump_ = "♣";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbNull, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Null;
      game_->trump_ = "";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbGrand, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Grand;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbRamsch, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Ramsch;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });

    QObject::connect(ui->pbHand, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->hand_ = checked;
                       qDebug() << "hand_ set to" << game_->hand_;
                     });

    QObject::connect(ui->pbOuvert, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->ouvert_ = checked;
                       qDebug() << "ouvert_ set to" << game_->ouvert_;
                     });
    QObject::connect(ui->pbSchneider, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->schneider_ = checked;
                       qDebug() << "schneider_ set to" << game_->schneider_;
                     });
    QObject::connect(ui->pbSchwarz, &QPushButton::toggled, this,
                     [this](bool checked) {
                       game_->schwarz_ = checked;
                       qDebug() << "schwarz_ set to" << game_->schwarz_;
                     });

    // Reizen
    QObject::connect(game_, &Game::bieten, this, &Table::onBieten);
    QObject::connect(game_, &Game::gehoert, this, &Table::onGehoert);

    QObject::connect(ui->pbSagen, &QPushButton::clicked, this, [this]() {
      game_->gereizt_ = game_->ansagen();
      // ui->pbSagen->setText(QString::number(game_->gereizt_));
      game_->sagen(game_->gereizt_);
      updateSkatLayout(true);
    });

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this, [this]() {
      qDebug() << "" << QString::fromStdString("");
    });

    QObject::connect(ui->pbDruecken, &QPushButton::clicked, this, [this]() {
      if (game_->skat_.cards().size() == 2) {
        // disconnect skat
        if (game_->rule_ != Rule::Ramsch)
          game_->getPlayerByIsSolo()->tricks_.push_back(
              std::move(game_->skat_));
        // connect all players to stick
        for (int playerId = 1; playerId <= 3; playerId++)
          updatePlayerLayout(playerId, 2);
        // update
        updateSkatLayout(false);
      }
    });

    QObject::connect(ui->pbStart, &QPushButton::clicked, game_, &Game::start);

    QObject::connect(game_, &Game::started, this, &Table::onStarted);
    // Connections
    QObject::connect(game_, &Game::clearTrickLayout, this,
                     &Table::onClearTrickLayout);
  }

  game_->init();
  game_->start();
}

void Table::updateSkatLayout(
    bool open) {
  QLayoutItem *item;

  while ((item = ui->gbSkatLayout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }
  for (Card &card : game_->skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    ui->gbSkatLayout->addWidget(cardButton);

    if (open) {
      Player *player = game_->getPlayerByIsSolo();
      int playerId = player->id();
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, this, cardButton, playerId]() {
            ui->gbSkatLayout->removeWidget(cardButton);
            cardButton->setParent(nullptr);
            game_->skat_.moveCardTo(std::move(card),
                                    game_->getPlayerByIsSolo()->handdeck_);
            qDebug() << game_->skat_.cards().size();
            qDebug() << game_->getPlayerByIsSolo()->handdeck_.cards().size();
            updatePlayerLayout(playerId, 1);
          });
    }
  }
}

void Table::updatePlayerLayout(
    int playerId, int dest) {
  Player &player = game_->getPlayerById(playerId);

  QLayout *layout;
  QLayoutItem *item;

  if (playerId == 1) layout = ui->gbPlayer1Layout;
  if (playerId == 2) layout = ui->gbPlayer2Layout;
  if (playerId == 3) layout = ui->gbPlayer3Layout;

  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }
  player.handdeck_.sortByJandSuits();

  for (Card &card : player.handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    layout->addWidget(cardButton);

    // connect to skat
    if (dest == 1) {
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, this, layout,
           cardButton]() {  // Bugfix: program crashed w/o layout, cardButton
            if (game_->skat_.cards().size() < 2) {
              layout->removeWidget(cardButton);
              cardButton->setParent(nullptr);
              player.handdeck_.moveCardTo(std::move(card), game_->skat_);
              updateSkatLayout(true);
            }
          });
    }

    // connect to trick
    if (dest == 2) {
      connect(
          cardButton, &QPushButton::clicked, this,
          [&, this, playerId, layout, cardButton]() {
            if (game_->playerList_.front()->id() == playerId) {
              if (!player.handdeck_.isCardInside(card) ||
                  !game_->isCardValid(card)) {
                qDebug() << "Move rejected: Invalid card choice.";
                return;
              }
              layout->removeWidget(cardButton);
              cardButton->setParent(nullptr);
              game_->playCard(
                  card);  // No modification, so this works with const reference

              // Insert the card button at the correct position
              if (playerId == 1) ui->gbTrickLayout->addWidget(cardButton, 0, 1);
              if (playerId == 2) ui->gbTrickLayout->addWidget(cardButton, 0, 0);
              if (playerId == 3) ui->gbTrickLayout->addWidget(cardButton, 0, 2);
            }
          });
    }
  }
}

void Table::updateTrickLayout() {
  QLayoutItem *item;

  while ((item = ui->gbTrickLayout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }
  for (Card &card : game_->trick_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    ui->gbSkatLayout->addWidget(cardButton);
  }
}

// Slots
void Table::onClearTrickLayout() {
  while (QLayoutItem *item = ui->gbTrickLayout->takeAt(0)) {
    if (QWidget *widget = item->widget()) {
      widget->deleteLater();  // Ensures safe deletion after event loop
    }
    delete item;  // Cleanup the layout item itself
  }
}

void Table::onStarted() {
  updateTrickLayout();
  updateSkatLayout(false);

  for (int playerId = 1; playerId <= 3; playerId++)
    updatePlayerLayout(playerId);
}

void Table::onBieten(
    int idSager, int idHoerer) {
  ui->pbSagen->click();
  // if (idSager == 1)
  //   ui->labelPlayer1->setText(QString::number(game_->gereizt_));
  if (idSager == 1)
    ui->pbSagen->setText(QString::number(game_->gereizt_));
  else
    ui->pbSagen->setText("");
  if (idSager == 2)
    ui->labelPlayer2->setText(QString::number(game_->gereizt_));
  else
    ui->labelPlayer2->setText("");
  if (idSager == 3)
    ui->labelPlayer3->setText(QString::number(game_->gereizt_));
  else
    ui->labelPlayer3->setText("");
  // QThread::msleep(1000);
}

void Table::onGehoert(
    int idHoerer, QString antwort) {
  if (idHoerer == 1)
    ui->pbSagen->setText("Höre");
  else
    ui->pbSagen->setText(QString::number(game_->gereizt_));
  if (idHoerer == 2) ui->labelPlayer2->setText(antwort);
  if (idHoerer == 3) ui->labelPlayer3->setText(antwort);
  // QThread::msleep(1000);
}

Table::~Table() { delete ui; }
