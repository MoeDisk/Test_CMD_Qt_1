#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QProcess>
#include <QDebug>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onExecuteClicked();
    void readProcessOutput();
    void readProcessError();
    void onInterruptClicked();

private:
    void startCmdProcess();
    void handleOutput();

    QPlainTextEdit *plainTextEdit;
    QLineEdit *lineEdit;
    QPushButton *executeButton;
    QPushButton *interruptButton;
    QProcess *cmdProcess;
};

#endif // MAINWINDOW_H
