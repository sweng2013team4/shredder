#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QtGui>
#include <QWidget>

namespace Ui {
    class Histogram;
}

class Histogram : public QWidget {
    Q_OBJECT

public:    
    Histogram(QString strPath);
    ~Histogram();
    void CalcHistogram();

protected:
    void resizeEvent(QResizeEvent *e);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

private:
    Ui::Histogram *ui;
    QString strPath;
    long long nFileSize;
    int nError;
    unsigned long long arrBytes[256];

    void LoadDisplay();

signals:
    void CloseSignal();
    void UpdateHistSignal(int nProgress);

public slots:
    void DrawProgress(int nProgress);
    void DrawHistogram();

private slots:
    void onResolutionChange();
};

#endif // HISTOGRAM_H
