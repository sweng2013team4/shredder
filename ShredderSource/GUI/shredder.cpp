
#include "shredder.h"
#include "ui_shredder.h"

#include <QtGui>
#include <string>
#include <iostream>

#include "helper.h"
#include "listdialog.h"
#include "aboutdialog.h"
#include "inputdialog.h"
#include "histthread.h"

using namespace std;

Shredder* Shredder::shredder = NULL;

Shredder::Shredder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Shredder)
{
    // Setup UI
    ui->setupUi(this);

    // Get desktop and connect to resolution change event
    QDesktopWidget* desktop = QApplication::desktop();
    connect(desktop, SIGNAL(resized(int)), this, SLOT(onResolutionChange()));

    // Initialization
    agMenuMode = new QActionGroup(ui->menuMode);

    ui->actBasic->setActionGroup(agMenuMode);
    ui->actAdvanced->setActionGroup(agMenuMode);
    ui->actLog->setActionGroup(agMenuMode);
    connect(agMenuMode, SIGNAL(triggered(QAction*)), this, SLOT(onMenuModeSelection(QAction*)));
    connect(ui->actExit, SIGNAL(triggered()), this, SLOT(close()));

    // Hide progress bar
    ui->prgScrubBar->setVisible(false);

    // Update disks display
    RefreshDisks();

    // Load pass type
    LoadPassTypes();

    // Load predefined patterns
    CreatePredefinedPatterns();

    // Center the window
    Helper::CenterWindow(this);
}

Shredder::~Shredder()
{
    delete ui;
}

Shredder* Shredder::GetInstance()
{
    if (shredder == NULL)
        shredder = new Shredder();

    return shredder;
}

void Shredder::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Shredder::on_btnRemove_clicked()
{
    QListWidgetItem* item = ui->lstFiles->takeItem(ui->lstFiles->currentRow());
    delete item;
}

void Shredder::on_btnBrowse_clicked()
{
    QString strPath;

    // Check browse mode
    switch(ui->cmbDataType->currentIndex())
    {
        // Add disks
        case DISK:
        {
            dmDisks.Refresh();
            QList<QListWidgetItem>* lstDisks = this->GetDiskList();
            ListDialog* dlgDisks = new ListDialog(this, QString::fromUtf8("בחירת דיסק"),
                                                      QString::fromUtf8("בחר דיסק למחיקה:"),
                                                      lstDisks);

            if (dlgDisks->exec() == QDialog::Accepted)
            {
                strPath = "/dev/" + lstDisks->at(dlgDisks->GetSelectedRow()).
                                                                data(Qt::UserRole).toString();
            }

            break;
        }
        // Add partitions
        case PARTITION:
        {
            dmDisks.Refresh();
            QList<QListWidgetItem>* lstParts = this->GetPartitionList();
            ListDialog* dlgParts = new ListDialog(this, QString::fromUtf8("בחירת מחיצה"),
                                                      QString::fromUtf8("בחר מחיצה למחיקה:"),
                                                      lstParts);

            if (dlgParts->exec() == QDialog::Accepted)
            {
                strPath = "/dev/" + lstParts->at(dlgParts->GetSelectedRow()).
                                                                data(Qt::UserRole).toString();
            }

            break;
        }
        // Add directories
        case DIRECTORY:
        {
            strPath = QFileDialog::getExistingDirectory(this,
                                                        QString::fromUtf8("בחר תיקיה למחיקה.."),
                                                        "/media",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
            break;
        }
        // Add files
        case FILE:
        {
            strPath = QFileDialog::getOpenFileName(this,
                                                   QString::fromUtf8("בחר קובץ למחיקה.."),
                                                   "/media",
                                                   "*");
            break;
        }
    }

    // Check if path is valid/already exists
    if(!strPath.isNull() &&
       ui->lstFiles->findItems(strPath, Qt::MatchFixedString).isEmpty())
    {
        // Add the path to the list
        ui->lstFiles->addItem(strPath);
    }
}

void Shredder::on_btnScrubBsc_clicked()
{
    if(!ui->tblDisks->currentIndex().isValid())
        return;

    int cr = ui->tblDisks->currentRow();

    // Check if the chosen drive is the boot drive
    if(ui->tblDisks->item(cr, 0)->data(Qt::UserRole) == BOOT_DEV_FLAG)
    {
        Helper::InfoBox(this,
                        "שגיאה",
                        "לא ניתן למחוק את הדיסק שממנו עלתה מערכת ההפעלה.");
        return;
    }

    QString strPath = "/dev/" + ui->tblDisks->item(cr, 0)->text();
    QString strQuestion = "האם אתה בטוח שאתה רוצה למחוק את הדיסק '" + strPath + "'?";

    bool bResult = Helper::QuestionBox(this,
                                      "מחיקת דיסק",
                                      strQuestion.toUtf8().data());

    if(bResult)
    {
        // Start shredding in basic mode
        Shred(true);
    }
}

void Shredder::on_btnRefresh_clicked()
{
    RefreshDisks();
}

void Shredder::RefreshDisks()
{
    int nRows;
    QString strBootDrive;

    // Clear table data
    ui->tblDisks->clearContents();

    // Refresh disk manager's list
    dmDisks.Refresh();

    nRows = dmDisks.DrivesCount;
    strBootDrive = dmDisks.GetBootDrive();

    // Set table rows
    ui->tblDisks->setRowCount(nRows);

    int nCurrRow = 0;

    // Fill each row by the drive info
    for (int i = 0; i < dmDisks.DrivesCount; i++)
    {
        QTableWidgetItem* itLabel =
                new QTableWidgetItem(dmDisks.GetDrive(i).DriveLabel, 0);
        QTableWidgetItem* itCapacity =
                new QTableWidgetItem(dmDisks.GetDrive(i).DriveCapacity, 0);
        QTableWidgetItem* itCylinders =
                new QTableWidgetItem(QString::number(dmDisks.GetDrive(i).DriveCylinders), 0);
        QTableWidgetItem* itHeads =
                new QTableWidgetItem(QString::number(dmDisks.GetDrive(i).DriveHeads), 0);
        QTableWidgetItem* itSectors =
                new QTableWidgetItem(QString::number(dmDisks.GetDrive(i).DriveSectorsPerTrack), 0);

        // Check if this is the boot drive
        if(strBootDrive == QString(dmDisks.GetDrive(i).DriveLabel))
        {
            // Set user role to 1 so no one can delete it
            itLabel->setData(Qt::UserRole, BOOT_DEV_FLAG);
            itLabel->setText("Boot - " + itLabel->text());
        }

        ui->tblDisks->setItem(nCurrRow, 0, itLabel);
        ui->tblDisks->setItem(nCurrRow, 1, itCapacity);
        ui->tblDisks->setItem(nCurrRow, 2, itCylinders);
        ui->tblDisks->setItem(nCurrRow, 3, itHeads);
        ui->tblDisks->setItem(nCurrRow, 4, itSectors);

        nCurrRow++;
    }
}

// This trigger clears file list when data type is changed
// Force selection of one type at a time
void Shredder::on_cmbDataType_currentIndexChanged(int nIndex)
{
    ui->lstFiles->clear();
}

/*
   Writes given string to UI log
*/
void Shredder::LogWriteUI(QString sMsg, bool bAlert, bool bAppend)
{
    QListWidgetItem* itWidget;

    // Check if we need to append and if it's possible
    if(bAppend && ui->lstLog->count())
    {
        // Get the last item and append string
        itWidget = ui->lstLog->item(ui->lstLog->count() - 1);
        itWidget->setText(itWidget->text() + " " + sMsg);
    }
    else
    {
        // Memory managed by parent
        itWidget = new QListWidgetItem(sMsg, ui->lstLog);
    }

    if(bAlert)
    {
        itWidget->setForeground(Qt::red);
    }
}

// Handles all events sent from thread
// Should get operation info from thread only - to be changed
void Shredder::HandleSignal(Logger::MSG_TYPE msgType, int nResult)
{
    QString sMsg;
    bool bAlert = false;
    bool bAppend = false;

    switch(msgType)
    {
        case Logger::SCRUB_START:
        {
            sMsg =  QDateTime::currentDateTime().toString(Qt::DefaultLocaleLongDate) + "\n" +
                    QString("מתחיל מחיקה: תבנית - %1, באפר - %2 KB\n");

            sMsg = sMsg.arg(QString::fromStdString(this->thrdShredder->scrubber->scrbPattern->sName)).
                        arg(this->thrdShredder->scrubber->nBufferSize / 1024);

            break;
        }
        case Logger::SCRUB_END:
        {
            QMessageBox::Icon icon;
            QString sInfo;

            // Just in-case for appearence
            ui->prgScrubBar->setValue(100);

            if (nResult == SCRUB_OK)
            {
                sInfo = "המחיקה הסתיימה בהצלחה!";
                icon = QMessageBox::Information;
            }
            else
            {
                sInfo = "המחיקה הסתיימה עם שגיאות!\n"
                        "למידע נוסף פנה לקובץ: ";

                sInfo += thrdShredder->scrubber->logger->sLogFilePath.c_str();

                icon = QMessageBox::Critical;
                bAlert = true;
            }

            // Print end time
            LogWriteUI(QDateTime::currentDateTime().toString(Qt::DefaultLocaleLongDate));

            // Print message add spacing
            sMsg = sInfo + "\n";

            // Display finish message
            Helper::InfoBox(this, "מחיקה הסתיימה", sInfo, icon);

            // Enable GUI
            this->SetBusyMode(false);

            // Remove thread
            delete thrdShredder;
            thrdShredder = NULL;

            break;
        }
        case Logger::SCRUB_FILE_START:
        {
            sMsg = QString("מוחק פריט: ") + this->thrdShredder->scrubber->lstFiles->at(nResult).c_str();

            break;
        }
        case Logger::SCRUB_PROGRESS:
        {
            ui->prgScrubBar->setValue(nResult);

            break;
        }
        case Logger::SCRUB_FILE_END:
        {
            if (nResult != SCRUB_OK)
            {
                sMsg = "מחיקת הפריט הסתיימה עם שגיאות!\n";
                bAlert = true;
            }
            else
            {
                sMsg = "מחיקת הפריט הסתיימה.\n";
            }

            break;
        }
        case Logger::SCRUB_PASS_START:
        {
            sMsg = "מתחיל מעבר";

            Pattern* patUsed = Shredder::GetPattern(thrdShredder->scrubber->scrbPattern->sName.c_str());

            // Just in-case..
            if (patUsed != NULL)
            {
                sMsg += " " + patUsed->lstPasses[nResult].toString();
            }

            sMsg += "...";

            break;
        }
        case Logger::SCRUB_PASS_END:
        {
            bAppend = true;

            if (nResult != SCRUB_OK)
            {
                sMsg = "שגיאה!";
                bAlert = true;
            }
            else
            {
                sMsg = "הסתיים";
            }

            break;
        }
        case Logger::SCRUB_VERIFY_START:
        {
            sMsg = "מבצע אימות...";

            break;
        }
        case Logger::SCRUB_VERIFY_END:
        {
            bAppend = true;

            if (nResult == SCRUB_OK)
            {
                sMsg = "בוצע";
            }
            else
            {
                bAlert = true;

                if (nResult == SCRUB_VERIFY_ERR)
                {
                    sMsg = "שגיאה - נתונים לא תואמים!";
                }
                else // SCRUB_ERR
                {
                    sMsg = "שגיאה - אימות לא בוצע!";
                }
            }

            break;
        }
        case Logger::SCRUB_ENTRY_START:
        {
            QString sFile = thrdShredder->scrubber->lstFiles->at(nResult).c_str();

            // Check if directory - reliable since '/' at the end is forced
            if(sFile.right(1) != "/")
                sMsg = "מסיר קובץ...";
            else
                sMsg = "מסיר תיקייה...";

            break;
        }
        case Logger::SCRUB_ENTRY_END:
        {
            bAppend = true;

            if (nResult != SCRUB_OK)
            {
                sMsg = "שגיאה!";
                bAlert = true;
            }
            else
            {
                sMsg = "הסתיים";
            }

            break;
        }
        case Logger::SCRUB_OPEN_ERROR:
        {
            sMsg = "שגיאה בפתיחת הקובץ!";
            bAlert = true;

            break;
        }
        default:
        {
            // do nothin ;(

            break;
        }
    }

    if (!sMsg.isEmpty())
        LogWriteUI(sMsg, bAlert, bAppend);
}

void Shredder::on_btnScrubAdv_clicked()
{
    // Check if a pattern is selected
    if(!ui->lstPatterns->currentIndex().isValid())
    {
        Helper::InfoBox(this,
                        "שגיאה",
                        "עליך לבחור תבנית בכדי לבצע מחיקה.");
        return;
    }

    QString strQuestion = QString::fromUtf8("האם אתה בטוח שאתה רוצה לבצע את המחיקה?");

    bool bResult = Helper::QuestionBox(this,
                                      "מחיקת מידע",
                                      strQuestion.toUtf8().data());
    if(bResult)
    {
        // Start shredding in advanced mode
        Shred(false);
    }
}

/*
   Recursively adds all files and directories within directory sPath
   NOTE: Will also add sPath and prevents dupes
*/
void Shredder::AddDirectoryContent(QString sPath, QList<QString>& lstFiles)
{
    QStringList lstEntries;
    QFileInfoList lstEntriesInfo;
    QDir dirPath(sPath);

    // Check sPath represents a directory
    if (dirPath.exists())
    {
        // Get directory list
        //lstEntries = dirPath.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

        lstEntriesInfo = dirPath.entryInfoList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDir::DirsFirst);

        if (sPath[sPath.count() - 1] != '/')
            sPath += '/';

        int i;

        // Add the content of each subdirectory aswell
        for (i = 0; i < lstEntriesInfo.count(); i++)
        {
            QString sEntry = sPath + lstEntriesInfo[i].fileName();
            bool bIsSymLink = lstEntriesInfo[i].isSymLink();

            if (bIsSymLink && !lstFiles.contains(sEntry))
            {
                lstFiles.append(sEntry);
            }
            else if (!bIsSymLink && !lstFiles.contains(sEntry + '/'))
            {
                AddDirectoryContent(sEntry + '/', lstFiles);
            }
        }

        // Get files list
        lstEntries = dirPath.entryList(QDir::Files | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);

        for (i = 0; i < lstEntries.count(); i++)
        {
            if (!lstFiles.contains(sPath + lstEntries[i]))
                lstFiles.append(sPath + lstEntries[i]);
        }
    }

    if (!lstFiles.contains(sPath))
    {
        // Add main directory/file
        lstFiles.append(sPath);
    }
}

// Kinda obsolete
void Shredder::LoadPatterns()
{
    ui->lstPatterns->clear();

    // Loop through list and load patterns to GUI
    for(int nCurr = 0; nCurr < lstPatterns.length(); nCurr++)
    {
        QListWidgetItem* itNew = new QListWidgetItem(ui->lstPatterns);
        itNew->setText(lstPatterns[nCurr].sName);
        itNew->setToolTip(itNew->text());
    }
}

void Shredder::AddPattern(Pattern* patNew)
{
    // Add to pattern list
    lstPatterns.append(*patNew);

    // Add to gui list widget
    QListWidgetItem* itNew = new QListWidgetItem(ui->lstPatterns);
    itNew->setText(patNew->sName);
    itNew->setToolTip(patNew->sName);
}

void Shredder::RemovePattern(int nIndex)
{
    Pattern& patToRemove = lstPatterns[nIndex];

    if(!patToRemove.bIsEditable)
    {
        Helper::InfoBox(this,
                        "שגיאה",
                        "התבנית לא ניתנת לעריכה.");
    }
    else
    {
        delete ui->lstPatterns->takeItem(nIndex);
        lstPatterns.removeAt(nIndex);
    }
}

void Shredder::CreatePredefinedPatterns()
{
    Pattern* patNew;

    // Basic
    patNew = new Pattern("Basic", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    AddPattern(patNew);

    // NSA Secure Flash
    patNew = new Pattern("NSA - Secure Flash", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::ONE);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ONE);
    AddPattern(patNew);

    // NSA Manual 130-2
    patNew = new Pattern("NSA Manual 130-2", false);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::STRING, "OK", false);
    patNew->AddPass(Pass::ZERO);
    AddPattern(patNew);

    // DoD - Fast Flash
    patNew = new Pattern("DoD - Fast Flash", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::ONE);
    AddPattern(patNew);

    // DoD 5220.22-M
    patNew = new Pattern("DoD 5220.22-M", false);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::ONE);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO, "0", true);
    AddPattern(patNew);

    // NSA + DoD - Top Secret Flash
    patNew = new Pattern("NSA + DoD - Top Secret Flash", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::ONE);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ONE);
    AddPattern(patNew);

    // Peter Guttman
    patNew = new Pattern("Peter Guttman", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::BYTE, "85", false);
    patNew->AddPass(Pass::BYTE, "170", false);
    patNew->AddPass(Pass::BYTE, "146,73,36", false);
    patNew->AddPass(Pass::BYTE, "73,36,146", false);
    patNew->AddPass(Pass::BYTE, "36,146,73", false);
    patNew->AddPass(Pass::BYTE, "0", false);
    patNew->AddPass(Pass::BYTE, "17", false);
    patNew->AddPass(Pass::BYTE, "34", false);
    patNew->AddPass(Pass::BYTE, "51", false);
    patNew->AddPass(Pass::BYTE, "68", false);
    patNew->AddPass(Pass::BYTE, "85", false);
    patNew->AddPass(Pass::BYTE, "102", false);
    patNew->AddPass(Pass::BYTE, "119", false);
    patNew->AddPass(Pass::BYTE, "136", false);
    patNew->AddPass(Pass::BYTE, "153", false);
    patNew->AddPass(Pass::BYTE, "170", false);
    patNew->AddPass(Pass::BYTE, "187", false);
    patNew->AddPass(Pass::BYTE, "204", false);
    patNew->AddPass(Pass::BYTE, "221", false);
    patNew->AddPass(Pass::BYTE, "238", false);
    patNew->AddPass(Pass::BYTE, "255", false);
    patNew->AddPass(Pass::BYTE, "146,73,36", false);
    patNew->AddPass(Pass::BYTE, "73,36,146", false);
    patNew->AddPass(Pass::BYTE, "36,146,73", false);
    patNew->AddPass(Pass::BYTE, "109,182,219", false);
    patNew->AddPass(Pass::BYTE, "182,219,109", false);
    patNew->AddPass(Pass::BYTE, "219,109,182", false);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::RAND);
    patNew->AddPass(Pass::ZERO);
    AddPattern(patNew);
}

QList<QListWidgetItem>* Shredder::GetDiskList()
{
    int i;
    QList<QListWidgetItem>* lstItems = new QList<QListWidgetItem>();

    QString strBootDrv = dmDisks.GetBootDrive();

    for(i = 0; i < dmDisks.DrivesCount; i++)
    {
        QListWidgetItem* itPart;
        disk_drive currDrive = dmDisks.GetDrive(i);

        QString strText = currDrive.DriveLabel;

        // Check that its not the boot drive
        if(strBootDrv != strText)
        {
            strText += " - " + QString::fromLatin1(currDrive.DriveCapacity);

            itPart = new QListWidgetItem(strText);
            itPart->setData(Qt::UserRole, currDrive.DriveLabel);
            lstItems->append(*itPart);
        }
    }

    return (lstItems);
}

QList<QListWidgetItem>* Shredder::GetPartitionList()
{
    int i,j;
    QList<QListWidgetItem>* lstItems = new QList<QListWidgetItem>();

    QString strBootPart = dmDisks.GetBootPart();

    for(i = 0; i < dmDisks.DrivesCount; i++)
    {
        QListWidgetItem* itPart;
        disk_drive currDrive = dmDisks.GetDrive(i);

        for(j = 0; j < dmDisks.GetDrive(i).DrivePartitionsCount; j++)
        {
            long long nSize = currDrive.DrivePartitions[j].PartitionSizeInBytes /1000;

            // Check if partition is extended, in that case ignore it
            if(nSize > 0)
            {
                QString strText = currDrive.DrivePartitions[j].PartitionLabel;

                // Check that its not the boot partition
                if(strBootPart != strText)
                {
                    strText += " - " + QString::number(nSize) + " MB";

                    itPart = new QListWidgetItem(strText);
                    itPart->setData(Qt::UserRole, currDrive.DrivePartitions[j].PartitionLabel);
                    lstItems->append(*itPart);
                }
            }
        }
    }

    return (lstItems);
}

Pattern* Shredder::GetPattern(QString sName)
{
    for(int i = 0; i < lstPatterns.count(); i++)
    {
        if(lstPatterns.at(i).sName == sName)
            return &lstPatterns[i];
    }

    return NULL;
}

void Shredder::on_btnDelPat_clicked()
{
    if(ui->lstPatterns->currentIndex().isValid())
    {
        RemovePattern(ui->lstPatterns->currentRow());
    }
}

void Shredder::on_lstPatterns_itemSelectionChanged()
{
    ui->lstPatEdit->clear();

    if(ui->lstPatterns->currentIndex().isValid())
    {
        LoadPatternInfo(lstPatterns.at(ui->lstPatterns->currentIndex().row()));
    }
}

void Shredder::LoadPassTypes()
{
    for(int nCurr = 0; nCurr < PASS_TYPES_NUM; nCurr++)
    {
        QListWidgetItem* itNew = new QListWidgetItem(ui->lstPatPasses);
        itNew->setText(Pass::PassTypes[nCurr]);
    }
}

void Shredder::LoadPatternInfo(Pattern patToLoad)
{
    for(int nCurr = 0; nCurr < patToLoad.lstPasses.length(); nCurr++)
    {
        QListWidgetItem* itNew = new QListWidgetItem(ui->lstPatEdit);
        QString sDisplay = (patToLoad.lstPasses[nCurr].toString(true));

        itNew->setText(sDisplay);
        itNew->setToolTip(sDisplay);
    }
}

void Shredder::SetBusyMode(bool bBusy)
{
    // Disable/Enable GUI
    ui->tabWidget->widget(Shredder::ADVANCED)->setDisabled(bBusy);
    ui->tabWidget->widget(Shredder::BASIC)->setDisabled(bBusy);

    // Show/Hide progress bar
    ui->prgScrubBar->setVisible(bBusy);

    if(bBusy)
    {
        ui->tabWidget->setCurrentIndex(Shredder::LOG);
        ui->actLog->setChecked(true);
    }
    else
    {
        ui->prgScrubBar->setValue(0);
    }
}


void Shredder::Shred(bool bBasicMode)
{
    SetBusyMode(true);

    this->thrdShredder = new ShredThread(CreateScrubber(bBasicMode));
    this->thrdShredder->start();
}

/*
   Creates scrubber object according to the current selected options
   Allocates memory for object, must be freed
*/
Scrubber* Shredder::CreateScrubber(bool bBasicMode)
{
    unsigned int nBufferSize;
    ScrubPattern* scrbPattern;

    vector<string>* lstFiles = new vector<string>();
/*    ScrubLogger* scrbLogger = new Logger(QString(LOGS_PATH) + QString(APP_NAME) + "-" +
                                         QDateTime::currentDateTime().toString("yyyy.MM.dd_HH.mm.ss"));*/
    ScrubLogger* scrbLogger = new Logger(QDateTime::currentDateTime().toString("yyyy.MM.dd_HH.mm.ss"));

    // Check mode
    if(bBasicMode)
    {
        // Add selected device to the list
        QString sDeviceName = ui->tblDisks->item(ui->tblDisks->currentRow(), 0)->text();
        lstFiles->push_back((QString(DEV_PATH) + sDeviceName).toStdString());

        scrbPattern = Shredder::GetPattern("Basic")->toScrubPattern();

        nBufferSize = DEFAULT_BUFFER_SIZE;
    }
    else
    {
        int i;
        QList<QString> lstFullList;

        // Get full files list
        for (i = 0; i < ui->lstFiles->count(); i++)
        {
            AddDirectoryContent(ui->lstFiles->item(i)->text(), lstFullList);
        }

        // Convert to std::list
        for (i = 0; i < lstFullList.count(); i++)
        {               
            lstFiles->push_back(lstFullList[i].toLocal8Bit().data());
        }

        // Get pattern
        scrbPattern = lstPatterns[ui->lstPatterns->currentRow()].toScrubPattern();

        nBufferSize = ui->spnBuffer->value() * 1024;
    }

    return (new Scrubber(lstFiles, scrbPattern, nBufferSize, scrbLogger));
}

void Shredder::on_btnAddPass_clicked()
{
    // Check if valid pattern and pass type are selected
    if(ui->lstPatterns->currentIndex().isValid() &&
       ui->lstPatPasses->currentIndex().isValid())
    {
        Pattern& pat = lstPatterns[ui->lstPatterns->currentRow()];

        if(!pat.bIsEditable)
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            "התבנית לא ניתנת לעריכה.");
        }
        else
        {
            Pass::PASS_TYPE pType = (Pass::PASS_TYPE)ui->lstPatPasses->currentRow();
            QString strVal = "0";

            if(pType == Pass::STRING || pType == Pass::BYTE)
            {
                InputDialog* dlgInput;

                if(pType == Pass::STRING)
                {
                    dlgInput = new InputDialog(this, "הוספת מעבר מחרוזת" ,"הכנס מחרוזת:", InputDialog::STRING);
                }
                else
                {
                    dlgInput = new InputDialog(this,
                                               "הוספת מעבר בתים",
                                               "הכנס בתים (מופרדים ע\"י ','):",
                                               InputDialog::NUMBERS,
                                               0, 255);
                }

                // Stop here if dialog was canceled
                if(dlgInput->exec() == QDialog::Rejected)
                {
                    return;
                }

                strVal = dlgInput->GetResult();
            }
            else if(pType == Pass::RAND && ui->cbVerifyPass->isChecked())
            {
                Helper::InfoBox(this,
                                "שגיאה",
                                "לא ניתן לאמת מעבר אקראי.");

                ui->cbVerifyPass->setChecked(false);

                return;
            }

            // Add pass to pattern
            pat.AddPass((Pass::PASS_TYPE)ui->lstPatPasses->currentRow(),
                         strVal,
                         ui->cbVerifyPass->isChecked());

            // Add pass to gui (instead of reloadin the whole thing)
            QListWidgetItem* itNew = new QListWidgetItem(ui->lstPatEdit);
            QString sDisplay = pat.lstPasses.last().toString(true);

            itNew->setText(sDisplay);
            itNew->setToolTip(sDisplay);

            // Set verify checkbox to unchecked
            ui->cbVerifyPass->setCheckState(Qt::Unchecked);
        }
    }
}

void Shredder::on_btnMoveUp_clicked()
{
    if(ui->lstPatterns->currentIndex().isValid() &&
       ui->lstPatEdit->currentIndex().isValid())
    {
        Pattern& pat = this->lstPatterns[ui->lstPatterns->currentRow()];

        if(!pat.bIsEditable)
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            "התבנית לא ניתנת לעריכה.");
        }
        else
        {
            int nIndex = ui->lstPatEdit->currentRow();

            if(nIndex-1 >= 0 && nIndex < pat.lstPasses.count())
            {
                // Move pass in gui list
                QListWidgetItem* itToMove = ui->lstPatEdit->takeItem(nIndex);
                ui->lstPatEdit->insertItem(nIndex-1, itToMove);
                ui->lstPatEdit->setCurrentRow(nIndex-1);

                // Move pass in pattern
                pat.MovePassUp(nIndex);
            }
        }
    }
}

void Shredder::on_btnMoveDown_clicked()
{
    if(ui->lstPatterns->currentIndex().isValid() &&
       ui->lstPatEdit->currentIndex().isValid())
    {
        Pattern& pat = this->lstPatterns[ui->lstPatterns->currentRow()];

        if(!pat.bIsEditable)
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            "התבנית לא ניתנת לעריכה.");
        }
        else
        {
            int nIndex = ui->lstPatEdit->currentRow();

            if(nIndex >= 0 && nIndex+1 < pat.lstPasses.count())
            {
                // Move pass in gui list
                QListWidgetItem* itToMove = ui->lstPatEdit->takeItem(nIndex);
                ui->lstPatEdit->insertItem(nIndex+1, itToMove);
                ui->lstPatEdit->setCurrentRow(nIndex+1);

                // Move pass in pattern
                pat.MovePassDown(nIndex);
            }
        }
    }
}

void Shredder::on_btnDelPass_clicked()
{
    if(ui->lstPatterns->currentIndex().isValid() &&
       ui->lstPatEdit->currentIndex().isValid())
    {
        Pattern& pat = this->lstPatterns[ui->lstPatterns->currentRow()];

        if(!pat.bIsEditable)
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            "התבנית לא ניתנת לעריכה.");
        }
        else
        {
            int nIndex = ui->lstPatEdit->currentRow();

            delete ui->lstPatEdit->takeItem(nIndex);
            pat.RemovePass(nIndex);
        }
    }
}

void Shredder::on_btnCreatePat_clicked()
{
    InputDialog* dlgInput = new InputDialog(this, "יצירת תבנית חדשה",
                                            "בחר שם לתבנית:",
                                            InputDialog::STRING);

    if(dlgInput->exec() == QDialog::Accepted)
    {
        // Create pattern
        Pattern* patNew = new Pattern(dlgInput->GetResult(), true);
        AddPattern(patNew);

        // Select the new pattern
        ui->lstPatterns->setCurrentRow(ui->lstPatterns->count() - 1);
    }
}

void Shredder::on_btnHistogram_clicked()
{
    if(!ui->tblDisks->currentIndex().isValid())
        return;

    QString strPath = "/dev/" +
                      ui->tblDisks->item(ui->tblDisks->currentRow(), 0)->text();


    Histogram* frmHist = new Histogram(strPath);
    HistThread* trdHistogram = new HistThread(frmHist);

    frmHist->show();

    trdHistogram->start(QThread::HighPriority);
}

void Shredder::onMenuModeSelection(QAction* actCurr)
{
    int nMode = agMenuMode->actions().indexOf(actCurr);

    bool bIsPermitted = false;

    if(nMode != ADVANCED)
    {
        bIsPermitted = true;
    }
    else
    {
        // retarded
        /*
        InputDialog* dlgInput = new InputDialog(this,
                                                "גישה חסומה",
                                                "הכנס סיסמא:",
                                                InputDialog::PASSWORD);

        if(dlgInput->exec() == QDialog::Accepted)
        {
            if(dlgInput->GetResult() == "123456")
            {
                bIsPermitted = true;
            }
            else
            {
                Helper::InfoBox(this,
                                "גישה חסומה",
                                "הסיסמא שהכנסת שגויה.");
            }
        }
        */

        bIsPermitted = true;
    }

    if(bIsPermitted)
        ui->tabWidget->setCurrentIndex(nMode);
    else
        (agMenuMode->actions()[ui->tabWidget->currentIndex()])->setChecked(true);
}

void Shredder::on_actVersion_triggered()
{
    AboutDialog dlgAbout;
    dlgAbout.exec();
}

void Shredder::on_btnResetLog_clicked()
{
    // It's possible to just connect clicked signal to list's clear() slot
    // but it's more intuitive to have a function since we have one for every button
    ui->lstLog->clear();
}

void Shredder::onResolutionChange()
{
    int nHeight, nWidth;

    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->screenGeometry();

    nHeight = this->height();
    nWidth = this->width();

    if(nHeight > rect.height())
        nHeight = rect.height();

    if(nWidth > rect.width())
        nWidth = rect.width();

    // Set the new window size
    this->resize(nWidth, nHeight*83/100);

    // Center the window
    Helper::CenterWindow(this);
}
