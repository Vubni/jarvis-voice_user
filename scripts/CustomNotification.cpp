#include "CustomNotification.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QStyle>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPainter>
#include <QIcon>
#include <QSvgRenderer>
#include <QLinearGradient>
#include <QRegularExpression>

QIcon createSvgIcon(const QString &path, int size) {
    QSvgRenderer renderer(path);
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    
    return QIcon(pixmap);
}

CustomNotification::CustomNotification(const QString& title, 
                                       const QString& message, 
                                       QWidget* parent,
                                       int displayTime)
    : QWidget(parent)
{
    // Настройка флагов окна
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    // Настройка стилей с увеличенными шрифтами
    setStyleSheet(R"(
        QLabel#title {
            color: #FFFFFF;
            font-weight: bold;
            font-size: 15px;
        }
        QLabel#message {
            color: #E0E0E0;
            font-size: 14px;
        }
    )");

    // Усиленная тень
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 200));
    shadow->setOffset(0, 6);
    setGraphicsEffect(shadow);

    // Увеличенные минимальные размеры
    setMinimumSize(330, 80);
    setMaximumWidth(500);

    setupUI(title, message);
    setupAnimation();

    int effectiveDisplayTime = displayTime;
    if (effectiveDisplayTime <= 0) {
        // Объединяем текст и удаляем HTML-теги
        QString fullText = title + " " + message;
        fullText.replace(QRegularExpression("<[^>]*>"), "");
        
        // Расчет времени на основе длины текста
        int wordCount = fullText.split(' ', Qt::SkipEmptyParts).count();
        effectiveDisplayTime = wordCount * 400; // 400ms на слово
        
        // Применяем минимальное время
        effectiveDisplayTime = qBound(3000, effectiveDisplayTime, 15000);
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CustomNotification::closeNotification);
    m_timer->start(effectiveDisplayTime);
}

void CustomNotification::setupUI(const QString& title, const QString& message)
{
    // Основной контейнер
    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet(R"(
        #container {
            background: qlineargradient(
                x1:0, y1:0.5, x2:1, y2:0.5,
                stop:0 #4A4E55, 
                stop:1 #5A5E65
            );
            border-radius: 8px;
            border: 1px solid #3C3F45;
        }
    )");
    
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);
    
    // Внутренний layout
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 5, 0); // Правый отступ 20px
    layout->setSpacing(0);

    // Контейнер для иконки (на всю высоту)
    QWidget* iconContainer = new QWidget(container);
    iconContainer->setFixedWidth(60);
    iconContainer->setStyleSheet("background: transparent;");
    
    QVBoxLayout* iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setSpacing(0);
    
    QLabel* iconLabel = new QLabel(iconContainer);
    iconLabel->setPixmap(createSvgIcon(":/images/logo.svg", 42).pixmap(42, 42));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("padding: 5px;");
    
    iconLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(iconContainer);

    // Разделительная линия
    QFrame* divider = new QFrame(container);
    divider->setFrameShape(QFrame::VLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setStyleSheet("background-color: #3C3F45; margin: 5px 0;");
    divider->setFixedWidth(1);
    layout->addWidget(divider);

    // Текстовый блок
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(12, 10, 0, 10);
    textLayout->setSpacing(2);  // Увеличенный интервал

    QLabel* titleLabel = new QLabel(title, container);
    titleLabel->setObjectName("title");
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    QLabel* msgLabel = new QLabel(message, container);
    msgLabel->setObjectName("message");
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    msgLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgLabel->setMinimumHeight(30); // Гарантированное место для текста
    
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(msgLabel);
    layout->addLayout(textLayout, 1); // Растягиваемый
}

void CustomNotification::setupAnimation()
{
    m_animation = new QPropertyAnimation(this, "windowOpacity", this);
    m_animation->setDuration(350);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::OutBack);
}

void CustomNotification::showNotification()
{
    adjustSize();
    
    // Позиционирование в правом нижнем углу с отступами
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = screenGeometry.width() - width() - 25;
    int y = screenGeometry.height() - height() - 25;
    move(x, y);

    show();
    m_animation->start();
}

void CustomNotification::closeNotification()
{
    // Плавное исчезновение
    QPropertyAnimation* closeAnim = new QPropertyAnimation(this, "windowOpacity", this);
    closeAnim->setDuration(300);
    closeAnim->setStartValue(1.0);
    closeAnim->setEndValue(0.0);
    closeAnim->setEasingCurve(QEasingCurve::InBack);
    
    connect(closeAnim, &QPropertyAnimation::finished, this, &CustomNotification::close);
    closeAnim->start();
}

void CustomNotification::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Градиентный фон
    QLinearGradient gradient(rect().topLeft(), rect().topRight());
    gradient.setColorAt(0, QColor(74, 78, 85));
    gradient.setColorAt(1, QColor(90, 94, 101));
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}