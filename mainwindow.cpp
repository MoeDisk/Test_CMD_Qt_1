#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTextCodec>
#include <QDebug>
#include <QShortcut>
#include <QKeySequence>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), cmdProcess(new QProcess(this))
{
    // 设置编码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM437"));

    // 创建控件
    plainTextEdit = new QPlainTextEdit(this);
    plainTextEdit->setPlaceholderText("Enter commands here...");
    plainTextEdit->setReadOnly(true);

    lineEdit = new QLineEdit(this);
    executeButton = new QPushButton("Execute", this);
    interruptButton = new QPushButton("Interrupt", this);

    // 布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(plainTextEdit);
    layout->addWidget(lineEdit);
    layout->addWidget(executeButton);
    layout->addWidget(interruptButton);

    setLayout(layout);

    // 连接信号和槽
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);
    connect(lineEdit, &QLineEdit::returnPressed, this, &MainWindow::onExecuteClicked);
    connect(interruptButton, &QPushButton::clicked, this, &MainWindow::onInterruptClicked);
    connect(cmdProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(cmdProcess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessError);

    // 创建并设置快捷键
    QShortcut *interruptShortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(interruptShortcut, &QShortcut::activated, this, &MainWindow::onInterruptClicked);

    // 启动cmd进程
    startCmdProcess();
}

MainWindow::~MainWindow()
{
    if (cmdProcess->state() == QProcess::Running) {
        cmdProcess->terminate();
        if (!cmdProcess->waitForFinished(1000)) { // 等待最多 1 秒
            cmdProcess->kill(); // 强制终止进程
            cmdProcess->waitForFinished();
        }
    }
}

void MainWindow::startCmdProcess()
{
    if (cmdProcess->state() == QProcess::NotRunning) {
        cmdProcess->start("cmd.exe");
        if (!cmdProcess->waitForStarted(1000)) { // 等待最多 1 秒
            qDebug() << "Failed to start cmd.exe";
        }
    }
}

void MainWindow::onExecuteClicked()
{
    QString command = lineEdit->text().trimmed(); // 获取用户输入的命令

    if (cmdProcess->state() == QProcess::NotRunning) {
        startCmdProcess(); // 启动cmd进程
    }

    if (!command.isEmpty()) {
        qDebug() << "Executing command:" << command; // 调试信息
        // 发送命令到cmd，包括回车符（\r\n）
        cmdProcess->write((command + "\r\n").toUtf8());
        cmdProcess->waitForBytesWritten(); // 等待内容写入
    } else {
        // 如果 lineEdit 为空，发送回车（空命令）
        cmdProcess->write("\r\n");
        cmdProcess->waitForBytesWritten(); // 等待内容写入
    }

    lineEdit->clear(); // 清空 lineEdit
}

void MainWindow::onInterruptClicked()
{
    qDebug() << "Interrupt button clicked"; // 调试信息
    if (cmdProcess->state() == QProcess::Running) {
        cmdProcess->write(QByteArrayLiteral("\x03")); // 发送 Ctrl+C (ASCII 0x03) 到 cmd 进程
        if (!cmdProcess->waitForBytesWritten(1000)) {
            qDebug() << "Failed to write interrupt signal.";
        }
        // 如果 Ctrl+C 信号没有生效，尝试终止进程
        if (cmdProcess->state() == QProcess::Running) {
            cmdProcess->terminate(); // 优雅地终止命令
            if (!cmdProcess->waitForFinished(1000)) {
                cmdProcess->kill(); // 强制终止进程
            }
            startCmdProcess(); // 重新启动cmd进程
        }
    }
}

void MainWindow::readProcessOutput()
{
    QByteArray output = cmdProcess->readAllStandardOutput();
    QString outputStr = QTextCodec::codecForLocale()->toUnicode(output);
    plainTextEdit->moveCursor(QTextCursor::End);
    plainTextEdit->appendPlainText(outputStr);

    // 限制输出显示的行数
    const int maxLines = 1000;
    int lines = plainTextEdit->blockCount();
    if (lines > maxLines) {
        plainTextEdit->setPlainText(plainTextEdit->toPlainText().split('\n').mid(lines - maxLines).join('\n'));
    }
}

void MainWindow::readProcessError()
{
    QByteArray error = cmdProcess->readAllStandardError();
    QString errorStr = QTextCodec::codecForLocale()->toUnicode(error);
    plainTextEdit->moveCursor(QTextCursor::End);
    plainTextEdit->appendPlainText("Error: " + errorStr); // 显示cmd错误
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 终止并等待cmdProcess完成
    if (cmdProcess->state() != QProcess::NotRunning) {
        cmdProcess->terminate();
        if (!cmdProcess->waitForFinished(100)) {
            cmdProcess->kill();
            cmdProcess->waitForFinished();
        }
    }
    event->accept(); // 接受关闭事件
}
