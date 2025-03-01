#include "table.h"

#include <QTimer>

#include "ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch), game_(new Game(this)) {
  ui->setupUi(this);
  game_->initGame();

  // connect pushbuttons
  {
    QObject::connect(ui->pbSagen, &QPushButton::clicked, this, [this]() {
      // game_->gereizt_ = game_->bieten();
      ui->pbSagen->setText(QString::number(game_->gereizt_));
    });
    // Reizen
    QObject::connect(game_, &Game::geboten, ui->pbSagen, &QPushButton::click);

    QObject::connect(ui->pbPassen, &QPushButton::clicked, this, [this]() {
      qDebug() << "" << QString::fromStdString("");
    });
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
    QObject::connect(ui->pbGrand, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Grand;
      game_->trump_ = "J";
      qDebug() << "trump_ set to" << QString::fromStdString(game_->trump_);
    });
    QObject::connect(ui->pbNull, &QPushButton::clicked, this, [this]() {
      game_->rule_ = Rule::Null;
      game_->trump_ = "";
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

    // Connections
    QObject::connect(game_, &Game::clearTrickLayout, this,
                     &Table::onClearTrickLayout);
  }

  for (int i = 0; i <= 3; i++) addCardsToLayout(i);
  // for (int i = 0; i <= 3; i++) connectCards(i);
  game_->startGame();
}

void Table::addCardsToLayout(
    int layoutId) {
  // Get the player by playerId
  Player *player = nullptr;
  QLayout *layout = nullptr;

  // Determine which player we're working with and their layout

  if (layoutId == 0) {
    // Skat cards handled separately
    for (const Card &card : game_->skat_.cards()) {
      QPushButton *cardButton = new QPushButton(this);
      cardButton->setText(QString::fromStdString(card.str()));
      ui->gbSkatLayout->addWidget(cardButton);
    }
    return;

  } else if (layoutId == 1) {
    player = &game_->player_1;
    layout = ui->gbPlayer1Layout;
  } else if (layoutId == 2) {
    player = &game_->player_2;
    layout = ui->gbPlayer2Layout;
  } else if (layoutId == 3) {
    player = &game_->player_3;
    layout = ui->gbPlayer3Layout;
  } else {
    qDebug() << "Invalid layoutID!";
    return;
  }

  for (const Card &card : player->handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(
        QString::fromStdString(card.str()));  // Set card text (UTF-8)
    layout->addWidget(cardButton);

    // Connect the QPushButton clicked signal to the corresponding lambda slot
    QObject::connect(cardButton, &QPushButton::clicked, this, [=, this]() {
      if (game_->playerList_.front()->id() == layoutId) {
        if (!player->handdeck_.isCardInside(card) ||
            !game_->isCardValid(card, game_->rule_)) {
          qDebug() << "Move rejected: Invalid card choice.";
          return;
        }
        layout->removeWidget(cardButton);
        cardButton->setParent(nullptr);  // Disconnect it from the layout
        game_->playCard(card);
        ui->gbTrickLayout->addWidget(cardButton);
      }
    });
    // qDebug() << "Connected cardButton for card:" << card.str().c_str();
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

Table::~Table() { delete ui; }
