#include "pictureset_dialog.h"
#include "ui_pictureset_dialog.h"
#include <QDebug>
PictureSet_Dialog::PictureSet_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PictureSet_Dialog)
{
    r_SendChannelValue=0;
    g_SendChannelValue=0;
    b_SendChannelValue=0;
    r_LastChannelValue=0;
    r_ChannelValue=0;
    g_LastChannelValue=0;
    g_ChannelValue =0;
    b_LastChannelValue=0;
    b_ChannelValue = 0;
    contrastValue = 0;//对比度滑动条当前值
    brightValue= 0;//亮度值滑动条当前值
    contrast_LastValue= 0;//对比度滑动条上一次值
    bright_LastValue= 0;//亮度滑动条上一次值
    contrast_SendValue= 0;//对比度发送值
    bright_SendValue = 0;
    ui->setupUi(this);
    this->setWindowTitle(tr("图像设置"));
    this->setWindowFlag(Qt::WindowContextHelpButtonHint,false);

}

PictureSet_Dialog::~PictureSet_Dialog()
{
    delete ui;
}

void PictureSet_Dialog::on_R_channelSlider_sliderMoved(int position)
{
    r_sliderEnable = true;
    r_ChannelValue = position;
    r_SendChannelValue = r_ChannelValue-r_LastChannelValue;
    QString R_channelValue;
    ui->R_channelDisp->setText(R_channelValue.setNum(position));


}

void PictureSet_Dialog::on_G_channerlSlider_sliderMoved(int position)
{
    g_sliderEnable = true;
    g_ChannelValue = position;
    g_SendChannelValue = g_ChannelValue-g_LastChannelValue;
    QString G_channelValue;
    ui->G_channelDisp->setText(G_channelValue.setNum(position));

}

void PictureSet_Dialog::on_B_channelSlider_sliderMoved(int position)
{
    b_sliderEnable = true;
    b_ChannelValue = position;
    b_SendChannelValue = b_ChannelValue-b_LastChannelValue;
    QString B_channelValue;
    ui->B_channelDisp->setText(B_channelValue.setNum(position));

}

void PictureSet_Dialog::on_ReturnBtn_clicked()
{
    returnSignal(5);
}

void PictureSet_Dialog::on_contrastSlider_sliderMoved(int position)
{
    contrast_Enable = true;
    contrastValue = position;
    contrast_SendValue = contrastValue-contrast_LastValue;
    QString ContrastValue_temp;
    ui->contrastDisp->setText(ContrastValue_temp.setNum(position));
}

void PictureSet_Dialog::on_brightSlider_sliderMoved(int position)
{
    bright_Enable = true;
    brightValue = position;
    bright_SendValue = brightValue-bright_LastValue;
    QString brightValue_temp;
    ui->brightDisp->setText(brightValue_temp.setNum(position));
}

void PictureSet_Dialog::on_WhiteBalanceBtn_clicked()
{
    emit sendSignalAutoWhiteBalance();
}

void PictureSet_Dialog::on_SetBtn_clicked()
{
    r_LastChannelValue=r_ChannelValue;
    g_LastChannelValue=g_ChannelValue;
    b_LastChannelValue=b_ChannelValue;
    contrast_LastValue = contrastValue;
    bright_LastValue = brightValue;
    if(r_sliderEnable ==false)
        r_SendChannelValue=0;
    if(g_sliderEnable ==false)
        g_SendChannelValue=0;
    if(b_sliderEnable ==false)
        b_SendChannelValue=0;
    if(contrast_Enable ==false)
        contrast_SendValue =0;
    if(bright_Enable ==false)
        bright_SendValue = 0;
    send_RGBGen(r_SendChannelValue,g_SendChannelValue,b_SendChannelValue);
    send_ContrastAndBright(contrast_SendValue,bright_SendValue);
    r_sliderEnable = false;
    g_sliderEnable = false;
    b_sliderEnable = false;
    contrast_Enable = false;
    bright_Enable = false;
}
