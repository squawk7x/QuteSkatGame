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
  QLayout *layout = nullptr;
  Player *player = nullptr;

  if (layoutId == 0) {
    addSkatCardsToLayout();
    return;
  }

  switch (layoutId) {
    case 1:
      player = &game_->player_1;
      layout = ui->gbPlayer1Layout;
      break;
    case 2:
      player = &game_->player_2;
      layout = ui->gbPlayer2Layout;
      break;
    case 3:
      player = &game_->player_3;
      layout = ui->gbPlayer3Layout;
      break;
    default:
      qDebug() << "Invalid layoutID!";
      return;
  }

  addPlayerCardsToLayout(player, layout, layoutId);
}

void Table::addPlayerCardsToLayout(
    Player *player, QLayout *layout, int layoutId) {
  for (const Card &card : player->handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    layout->addWidget(cardButton);

    connect(cardButton, &QPushButton::clicked, this, [=, this]() {
      if (game_->playerList_.front()->id() == layoutId) {
        if (!player->handdeck_.isCardInside(card) ||
            !game_->isCardValid(card, game_->rule_)) {
          qDebug() << "Move rejected: Invalid card choice.";
          return;
        }
        layout->removeWidget(cardButton);
        cardButton->setParent(nullptr);
        game_->playCard(card);
        ui->gbTrickLayout->addWidget(cardButton);
      }
    });
  }
}

void Table::addSkatCardsToLayout() {
  QLayoutItem *item;

  while ((item = ui->gbSkatLayout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }
  for (const Card &card : game_->skat_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    ui->gbSkatLayout->addWidget(cardButton);

    connect(cardButton, &QPushButton::clicked, this, [=, this]() {
      game_->skat_.moveCardTo(card, game_->getPlayerByIsSolo()->handdeck_);
      qDebug() << game_->getPlayerByIsSolo()->handdeck_.cards().size();
      ui->gbSkatLayout->removeWidget(cardButton);
      cardButton->setParent(nullptr);
      updatePlayerHanddeckLayout();
    });
  }
}

void Table::updatePlayerHanddeckLayout() {
  Player *player = game_->getPlayerByIsSolo();
  int playerId = player->id();

  // Clear the existing layout by removing all widgets
  QLayout *layout;
  if (playerId == 1) layout = ui->gbPlayer1Layout;
  if (playerId == 2) layout = ui->gbPlayer2Layout;
  if (playerId == 3) layout = ui->gbPlayer3Layout;
  QLayoutItem *item;

  // Loop through all items in the layout and remove them
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      delete item->widget();  // Delete the widget (if it exists)
    }
    delete item;  // Delete the layout item
  }

  // Add the cards to the player's layout
  for (const Card &card : player->handdeck_.cards()) {
    QPushButton *cardButton = new QPushButton(this);
    cardButton->setText(QString::fromStdString(card.str()));
    layout->addWidget(cardButton);

    // Connect the card button to the necessary slot, if needed
    connect(cardButton, &QPushButton::clicked, this, [=, this]() {
      if (game_->skat_.cards().size() < 2) {
        layout->removeWidget(cardButton);
        cardButton->setParent(nullptr);
        player->handdeck_.moveCardTo(card, game_->skat_);
        // ui->gbSkatLayout->addWidget(cardButton);
        addSkatCardsToLayout();
      }
    });
  }
}
void Table::onUpdateSkat() { qDebug() << "updating Skat...!"; }
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
