#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void onReadyRead();
    void adjustInputTextEditSize();
    void adjustResponseTextEditSize();

private:
    QString formatJsonValue(const QJsonValue &value, int indentLevel = 0);
    void adjustTextEditSize(QTextEdit *textEdit);

    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
};

#endif // MAINWINDOW_H
