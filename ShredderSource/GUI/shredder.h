#ifndef SHREDDER_H
#define SHREDDER_H

#include <QtGui>
#include <string>

#include "DiskManager/diskmanager.h"
#include "pattern.h"
#include "logger.h"
#include "shredthread.h"

#define APP_NAME    "Shredder"
#define DEV_PATH    "/dev/"
#define LOGS_PATH   "/home/shredder/"

#define DEFAULT_BUFFER_SIZE 1024 * 1024
#define BOOT_DEV_FLAG   5

namespace Ui {
    class Shredder;
}

class Shredder : public QMainWindow {
    Q_OBJECT

public:
    // Enums
    enum WORK_MODE
    {
        BASIC,
        ADVANCED,
        LOG
    };

    enum DATA_TYPE
    {
        DISK,
        PARTITION,
        DIRECTORY,
        FILE
    };

    enum WRITE_TYPE
    {
        RANDOM,
        STRING,
        BYTE
    };

    static Shredder* GetInstance();

private:
    // Instance
    static Shredder* shredder;

    // Ctor/Dtor
    Shredder(QWidget *parent = 0);
    ~Shredder();

    // Data Variables
    DiskManager dmDisks;
    QList<Pattern> lstPatterns;
    ShredThread* thrdShredder;

    // UI Variables
    QActionGroup* agMenuMode;
    Ui::Shredder* ui;

    // UI Functions
    void RefreshDisks();
    void LoadPatterns();
    void AddPattern(Pattern* patNew);
    void RemovePattern(int nIndex);
    void LoadPassTypes();
    void LoadPatternInfo(Pattern patToLoad);
    void SetBusyMode(bool bBusy);
    void LogWriteUI(QString sMsg, bool bAlert = false, bool bAppend = false);

    // Other Member functions
    void Shred(bool bBasicMode);
    Scrubber* CreateScrubber(bool bBasicMode);
public:
    void AddDirectoryContent(QString sPath, QList<QString>& lstFiles);
private:
    void CreatePredefinedPatterns();

    QList<QListWidgetItem>* GetPartitionList();
    QList<QListWidgetItem>* GetDiskList();
    Pattern* GetPattern(QString strName);

    void changeEvent(QEvent *e);

public slots:
    void HandleSignal(Logger::MSG_TYPE msgType, int nResult);

private slots:
    void onResolutionChange();
    void on_btnResetLog_clicked();
    void on_actVersion_triggered();
    void on_btnHistogram_clicked();
    void on_btnCreatePat_clicked();
    void on_btnDelPass_clicked();
    void on_btnMoveDown_clicked();
    void on_btnMoveUp_clicked();
    void on_btnAddPass_clicked();
    void on_lstPatterns_itemSelectionChanged();
    void on_btnDelPat_clicked();
    void on_btnScrubAdv_clicked();
    void on_cmbDataType_currentIndexChanged(int index);
    void on_btnRefresh_clicked();
    void on_btnScrubBsc_clicked();
    void on_btnBrowse_clicked();
    void on_btnRemove_clicked();
    void onMenuModeSelection(QAction* actCurr);
};

#endif // SHREDDER_H
