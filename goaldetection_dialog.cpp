#include "goaldetection_dialog.h"
#include "ui_goaldetection_dialog.h"
#include <QDebug>
GoalDetection_Dialog::GoalDetection_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GoalDetection_Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("目标识别设定"));
    this->setWindowFlag(Qt::WindowContextHelpButtonHint,false);
    m_basketBallRobot = new BasketBallRobotModeSetWidget;
    m_roadRobot = new RoadRobotModeSetWidget;
    ui->verticalLayout->addWidget(m_roadRobot);
    currentWidget = m_roadRobot->objectName();
    RobotMode = Mode_RoadRobot;
}

GoalDetection_Dialog::~GoalDetection_Dialog()
{
    delete ui;
}

void GoalDetection_Dialog::on_returnBtn_clicked()
{
    returnSignal(4);
}

void GoalDetection_Dialog::on_comboBox_currentIndexChanged(int index)
{
    index = index+1;
    switch (index) {
    case Mode_RoadRobot:
        ui->verticalLayout->removeWidget(returnCurrentWidget(currentWidget));
         returnCurrentWidget(currentWidget)->close();
        m_roadRobot = new RoadRobotModeSetWidget;
        RobotMode = Mode_RoadRobot;
        //send_RobotMode(RobotMode);
        ui->verticalLayout->addWidget(m_roadRobot);
         currentWidget = m_roadRobot->objectName();
        break;
    case Mode_BastBallRobot:
        ui->verticalLayout->removeWidget(returnCurrentWidget(currentWidget));
        returnCurrentWidget(currentWidget)->close();
        m_basketBallRobot = new BasketBallRobotModeSetWidget;
        RobotMode = Mode_BastBallRobot;
        //send_RobotMode(RobotMode);
        ui->verticalLayout->addWidget(m_basketBallRobot);
        currentWidget = m_basketBallRobot->objectName();
        break;
    default:
        break;
    }
}

QWidget* GoalDetection_Dialog::returnCurrentWidget(QString currentWidgetName)
{
  if(currentWidgetName==m_roadRobot->objectName())
  {
      return m_roadRobot;
  }
  else if(currentWidgetName ==m_basketBallRobot->objectName())
  {
      return m_basketBallRobot;
  }
  return NULL;
}

void GoalDetection_Dialog::on_SetBtn_clicked()
{
    qDebug()<<"ori_mode"<<RobotMode<<endl;
     send_RobotMode(RobotMode);
}
