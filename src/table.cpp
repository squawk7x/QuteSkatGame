#include "table.h"

#include "./ui_table.h"

Table::Table(
    QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Skattisch) {
  ui->setupUi(this);

  game_.initGame();

  // Setup Layout Players
  {
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
            if (game_.playerList_.front()->id() == game_.player_1.id()) {
              // ✅ First, check if card in hand and the move is valid
              if (!game_.player_1.handdeck_.isCardInside(card) ||
                  !game_.isCardValid(card, game_.rule_)) {
                qDebug() << "Move rejected: Invalid card choice.";
                return;  // 🚨 Exit early if the move is not valid
              }

              // ✅ Only remove the card if it's valid
              gbPlayer1Layout->removeWidget(cardButton);
              cardButton->setParent(
                  nullptr);  // Remove parent to avoid conflicts

              // ✅ Play the card
              game_.playCard(card);

              // ✅ Move card to trick only after it has been successfully
              // played
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
            if (game_.playerList_.front()->id() == game_.player_2.id()) {
              // ✅ First, check if card in hand and the move is valid
              if (!game_.player_2.handdeck_.isCardInside(card) ||
                  !game_.isCardValid(card, game_.rule_)) {
                qDebug() << "Move rejected: Invalid card choice.";
                return;  // 🚨 Exit early if the move is not valid
              }

              // ✅ Only remove the card if it's valid
              gbPlayer2Layout->removeWidget(cardButton);
              cardButton->setParent(
                  nullptr);  // Remove parent to avoid conflicts

              // ✅ Play the card
              game_.playCard(card);

              // ✅ Move card to trick only after it has been successfully
              // played
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
            if (game_.playerList_.front()->id() == game_.player_3.id()) {
              // ✅ First, check if card in hand and the move is valid
              if (!game_.player_3.handdeck_.isCardInside(card) ||
                  !game_.isCardValid(card, game_.rule_)) {
                qDebug() << "Move rejected: Invalid card choice.";
                return;  // 🚨 Exit early if the move is not valid
              }

              // ✅ Only remove the card if it's valid
              gbPlayer3Layout->removeWidget(cardButton);
              cardButton->setParent(
                  nullptr);  // Remove parent to avoid conflicts

              // ✅ Play the card
              game_.playCard(card);

              // ✅ Move card to trick only after it has been successfully
              // played
              gbTrickLayout->addWidget(cardButton);
            }
          });
    }
  }

  // Pushbuttons
  {
    QPushButton *pbKaro = findChild<QPushButton *>("pbKaro");
    QPushButton *pbHerz = findChild<QPushButton *>("pbHerz");
    QPushButton *pbPik = findChild<QPushButton *>("pbPik");
    QPushButton *pbKreuz = findChild<QPushButton *>("pbKreuz");
    QPushButton *pbGrand = findChild<QPushButton *>("pbGrand");
    QPushButton *pbNull = findChild<QPushButton *>("pbNull");
    QPushButton *pbRamsch = findChild<QPushButton *>("pbRamsch");
    QPushButton *pbOuvert = findChild<QPushButton *>("pbOuvert");
    QPushButton *pbHand = findChild<QPushButton *>("pbHand");
    QPushButton *pbSchneider = findChild<QPushButton *>("pbSchneider");
    QPushButton *pbSchwarz = findChild<QPushButton *>("pbSchwarz");

    // connect pushbuttons Spiel
    {
      QObject::connect(pbKaro, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "♦";
        game_.rule_ = Rule::Suit;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbHerz, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "♥";
        game_.rule_ = Rule::Suit;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbPik, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "♠";
        game_.rule_ = Rule::Suit;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbKreuz, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "♣";
        game_.rule_ = Rule::Suit;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbGrand, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "J";
        game_.rule_ = Rule::Grand;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbNull, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "";
        game_.rule_ = Rule::Null;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
      QObject::connect(pbRamsch, &QPushButton::clicked, this, [this]() {
        game_.trump_ = "J";
        game_.rule_ = Rule::Ramsch;
        qDebug() << "trump_ set to " << QString::fromStdString(game_.trump_);
      });
    }

    {
      QObject::connect(pbHand, &QPushButton::toggled, this,
                       [this](bool checked) {
                         game_.hand_ = checked;
                         qDebug() << "hand_ set to " << game_.hand_;
                       });

      QObject::connect(pbOuvert, &QPushButton::toggled, this,
                       [this](bool checked) {
                         game_.ouvert_ = checked;
                         qDebug() << "ouvert_ set to " << game_.ouvert_;
                       });
      QObject::connect(pbSchneider, &QPushButton::toggled, this,
                       [this](bool checked) {
                         game_.schneider_ = checked;
                         qDebug() << "schneider_ set to " << game_.schneider_;
                       });
      QObject::connect(pbSchwarz, &QPushButton::toggled, this,
                       [this](bool checked) {
                         game_.schwarz_ = checked;
                         qDebug() << "schwarz_ set to " << game_.schwarz_;
                       });
    }
  }

  // Connections
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
