#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QScreen>
#include <QGuiApplication>
#include <QDateTime>
#include <QTimer>  // Thêm thư viện QTimer

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);

    // Set the stylesheet for the entire MainWindow
    this->setStyleSheet("QWidget { background-color: white; color: black; }");

    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::on_sendButton_clicked);

    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);

    // Connect signals to adjust size of text edits
    connect(ui->inputTextEdit->document(), &QTextDocument::contentsChanged, this, &MainWindow::adjustInputTextEditSize);
    connect(ui->responseTextEdit->document(), &QTextDocument::contentsChanged, this, &MainWindow::adjustResponseTextEditSize);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    if (!ui->sendButton->isEnabled())
        return;
    ui->sendButton->setEnabled(false); // Vô hiệu hóa nút

    // Lấy thời gian hiện tại
    QDateTime currentTime = QDateTime::currentDateTime();
    QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz"); // Định dạng thời gian với milliseconds

    // Log thời gian
    qDebug() << "Button clicked at:" << formattedTime;

    QString message = ui->inputTextEdit->toPlainText();
    qDebug() << "Message sent to server:" << message.toUtf8();

    tcpSocket->connectToHost(QHostAddress::LocalHost, 8080);
    if (tcpSocket->waitForConnected())
    {
        tcpSocket->write(message.toUtf8());
    }
    else
    {
        ui->responseTextEdit->setText("Failed to connect to server");
    }
    QTimer::singleShot(1000, this, [this]() {
        ui->sendButton->setEnabled(true); // Kích hoạt lại nút sau 1 giây
    });
}

void MainWindow::onReadyRead()
{
    QByteArray response = tcpSocket->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(response, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        ui->responseTextEdit->setText("Invalid JSON response: " + parseError.errorString());
        return;
    }

    QString formattedResponse = formatJsonValue(jsonResponse.isObject() ? QJsonValue(jsonResponse.object()) : QJsonValue(jsonResponse.array()));
    ui->responseTextEdit->setText(formattedResponse);
}

QString MainWindow::formatJsonValue(const QJsonValue &value, int indentLevel)
{
    QString formattedString;
    QString indent(indentLevel * 2, ' ');

    if (value.isObject())
    {
        QJsonObject obj = value.toObject();
        formattedString += "{\n";
        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            formattedString += indent + "  \"" + it.key() + "\": " + formatJsonValue(it.value(), indentLevel + 1) + "\n";
        }
        formattedString += indent + "}";
    }
    else if (value.isArray())
    {
        QJsonArray array = value.toArray();
        formattedString += "[\n";
        for (const QJsonValue &val : array)
        {
            formattedString += indent + "  " + formatJsonValue(val, indentLevel + 1) + "\n";
        }
        formattedString += indent + "]";
    }
    else if (value.isString())
    {
        formattedString += "\"" + value.toString() + "\"";
    }
    else if (value.isDouble())
    {
        formattedString += QString::number(value.toDouble());
    }
    else if (value.isBool())
    {
        formattedString += value.toBool() ? "true" : "false";
    }
    else if (value.isNull())
    {
        formattedString += "null";
    }

    return formattedString;
}

void MainWindow::adjustInputTextEditSize()
{
    adjustTextEditSize(ui->inputTextEdit);
}

void MainWindow::adjustResponseTextEditSize()
{
    adjustTextEditSize(ui->responseTextEdit);
}

void MainWindow::adjustTextEditSize(QTextEdit *textEdit)
{
    // Lấy chiều cao tối đa là một nửa chiều cao màn hình
    int maxHeight = QGuiApplication::primaryScreen()->availableGeometry().height() / 2;
    int contentHeight = textEdit->document()->size().height() + 10; // Chiều cao của nội dung

    if (contentHeight > maxHeight)
    {
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // Bật thanh cuộn khi cần
        textEdit->setFixedHeight(maxHeight); // Cố định chiều cao tối đa
    }
    else
    {
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Tắt thanh cuộn nếu không cần
        textEdit->setFixedHeight(contentHeight); // Đặt chiều cao dựa trên nội dung
    }
}
