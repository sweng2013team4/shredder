 #include "histogram.h"
#include "ui_histogram.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>

#include "Scrubber/utility.h"
#include "helper.h"


using namespace std;

Histogram::Histogram(QString strPath) :
    QWidget(0),
    ui(new Ui::Histogram)
{
    ui->setupUi(this);

    // Get desktop and connect to resolution change event
    QDesktopWidget* desktop = QApplication::desktop();
    connect(desktop, SIGNAL(resized(int)), this, SLOT(onResolutionChange()));

    // Set DM
    this->strPath = strPath;
    this->nError = 0;

    // Set title
    this->setWindowTitle(QString("היסטוגרמה - ") + strPath);

    // Hide labels
    ui->lblCommon->hide();

    // Center the window
    Helper::CenterWindow(this);
}

Histogram::~Histogram()
{
    disconnect(this, 0);
    delete ui;
}

void Histogram::resizeEvent(QResizeEvent *e)
{
    /*
    int nBarHeight;
    int nMaxWinHeightPct = 70;
    int nPortHeight = ui->frmBg->height() / 100. * nMaxWinHeightPct;

    unsigned long long nTopLine;

    // Get highest histogram value
    nTopLine = arrBytes[0];

    for(int i = 1; i < 256; i++)
    {
        if(nTopLine < arrBytes[i])
        {
            nTopLine = arrBytes[i];
        }
    }

    for(int i = 0; i < 256; i++)
    {

        QWidget* widget = ui->frmBg->findChild<QWidget *>("Bar" + QString::number(i));

        if(widget)
        {
            nBarHeight = nPortHeight / 100. *((double)arrBytes[i]/nTopLine * 100);
            widget->setMinimumHeight(nBarHeight);
        }
    }

    this->update();
    */
}

void Histogram::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Histogram::closeEvent(QCloseEvent *e)
{
    emit CloseSignal();
    e->accept();
}

void Histogram::DrawHistogram()
{
    this->hide();

    ui->prgHistBar->hide();

    if (nError == 0)
    {
        //Show histogram graph labels
        ui->lblCommon->show();
        ui->lblVal->show();

        //Hide background and message
        ui->widget->hide();
        ui->lblMessage->hide();

        // Draw historgram
        this->LoadDisplay();
    }
    else
    {
        ui->lblMax->hide();
        ui->lblMin->hide();
        ui->lblVal->hide();
        ui->line->hide();
        ui->lblMessage->setText("שגיאה בבניית ההיסטוגרמה. פנה לצוות תמיכה.");
    }

    this->update();
    this->show();
}

void Histogram::DrawProgress(int nProgress)
{
    ui->prgHistBar->setValue(nProgress);
}

void Histogram::CalcHistogram()
{  
    long long nBufferSize = 1024*1024*5;

    this->nFileSize = Utility::GetFileSize(strPath.toUtf8().data());

    if (this->nFileSize == -1)
    {
        this->nError = 1;
        return;
    }

    // Init position and buffer
    long long nCurrPos = 0;
    unsigned char* buffer = new unsigned char[nBufferSize];

    // Reset array
    memset(this->arrBytes, 0, sizeof(long long) * 256);

    // Open file
    int fp = open(strPath.toUtf8().data() ,  O_RDONLY);

    if (fp)
    {
        double dPct = 0;
        int nRead;

        while (nCurrPos < nFileSize && nError == 0)
        {
            if (nCurrPos + nBufferSize > nFileSize)
                nBufferSize = nFileSize - nCurrPos;

            // Read to buffer
            nRead = Utility::ReadData(fp, buffer, nBufferSize);

            if(nRead == -1)
            {
                nError = 2;
            }
            else
            {
                // Add to histogram by buffer
                for (long long i = 0; i < nBufferSize; i++)
                {
                    this->arrBytes[buffer[i]]++;
                }

                // Progress our position
                nCurrPos += nBufferSize;

                dPct = nCurrPos / (double)nFileSize * 100;

                emit UpdateHistSignal((int)dPct);
            }
        }

        ::close(fp);
    }

    //for(int i = 0; i < 256; i++)
    //    this->arrBytes[i] = rand()%10000000;

    delete[] buffer;
}

void Histogram::LoadDisplay()
{
    unsigned long long nTopLine;

    if (this->nError != 0)
    {
        return;
    }

    // Get highest histogram value
    nTopLine = arrBytes[0];

    for(int i = 1; i < 256; i++)
    {
        if(nTopLine < arrBytes[i])
        {
            nTopLine = arrBytes[i];
        }
    }

    // Create background layout for the graph bars
    QHBoxLayout* topLayout = (QHBoxLayout*)ui->frmBg->layout();

    topLayout->setSpacing(1);

    int nBarHeight;
    float fBytePct; // Byte appearence precent
    int nMaxWinHeightPct = 60;
    int nPortHeight = ui->frmBg->height() / 100. * nMaxWinHeightPct;

    // Create the bars and their layouts
    for(int i = 0; i < 256; i++)
    {
        QVBoxLayout* layout = new QVBoxLayout();

        layout->addSpacerItem(new QSpacerItem(2, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

        QWidget* widget = new QWidget(ui->frmBg);

        // Calculate bar height and pct of appearences
        fBytePct = (double)arrBytes[i]/this->nFileSize * 100;

        nBarHeight = nPortHeight / 100. *((double)arrBytes[i]/nTopLine * 100);

        if(fBytePct > 0.)
        {
            nBarHeight = max(nBarHeight, nPortHeight/6);
            nBarHeight = max(nBarHeight, 20);
        }

        widget->setObjectName("Bar" + QString::number(i));
        widget->setMinimumSize(2, nBarHeight);
        widget->setMouseTracking(true);
        widget->setStyleSheet("background-color: rgb(0, 135, 202);");
        widget->setToolTip(QString("Byte: %1 - %2%").arg(i).arg(fBytePct, 0, 'f', 2));
        //widget->setStatusTip(QString("הבית %1 מופיע %2 פעמים.").arg(i).arg(arrBytes[i]));

        layout->addWidget(widget);

        topLayout->addLayout(layout);
    }
}

void Histogram::onResolutionChange()
{
    int nHeight, nWidth;

    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->screenGeometry();

    QMessageBox::information(this,
                             "Info",
                             "Width: " + QString::number(this->width()) +
                             " Height: " + QString::number(this->height()));

    nHeight = this->height();
    nWidth = this->width();

    if(nHeight > rect.height())
        nHeight = rect.height();

    if(nWidth > rect.width())
        nWidth = rect.width();

    // Set the new window size
    this->resize(nWidth, nHeight*83/100);

    QMessageBox::information(this,
                             "Info",
                             "Width: " + QString::number(this->width()) +
                             " Height: " + QString::number(this->height()));



    // Center the window
    Helper::CenterWindow(this);
}
