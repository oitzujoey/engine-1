#include "runnerwindow.h"
#include "./ui_runnerwindow.h"

RunnerWindow::RunnerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RunnerWindow)
{
    ui->setupUi(this);
}

RunnerWindow::~RunnerWindow()
{
    delete ui;
}

