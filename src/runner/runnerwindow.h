#ifndef RUNNERWINDOW_H
#define RUNNERWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class RunnerWindow; }
QT_END_NAMESPACE

class RunnerWindow : public QMainWindow
{
    Q_OBJECT

public:
    RunnerWindow(QWidget *parent = nullptr);
    ~RunnerWindow();

private:
    Ui::RunnerWindow *ui;
};
#endif // RUNNERWINDOW_H
