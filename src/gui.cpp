#include "gui.h"
#include "ui_gui.h"

Gui::Gui(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Gui),
    currentArchive(new Archive()){

    ui->setupUi(this);
    setAcceptDrops(true);

    ui_openbutton = findChild<QPushButton *>("openButton");
    ui_saveButton = findChild<QPushButton *>("saveButton");
    ui_discardButton = findChild<QPushButton *>("discardButton");
    fileList = findChild<QListWidget *>("listFiles");
    summaryLabel = findChild<QLabel *>("summaryLabel");
}

Gui::~Gui(){
    delete ui;
}

void Gui::resetWindow(){
    summaryLabel->setText("0 files | Packed size: 0B");
    ui_openbutton->setEnabled(true);
    ui_openbutton->setText("Open archive");
    ui_saveButton->setText("Save archive");
    ui_saveButton->setEnabled(false);
    ui_discardButton->setEnabled(false);
    fileList->clear();

    currentArchive.reset(new Archive());
}

void Gui::on_openButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("riv archive (*.riv)"));

    if(fileName.isEmpty()){
        return;
    }
    ui_openbutton->setEnabled(false);
    ui_discardButton->setEnabled(true);

    currentArchive.reset(new Archive(fileName));

    QFileInfo fileInfo(fileName);
    QString title(fileInfo.fileName());
    ui_openbutton->setText(title);

    loadFilesList();
}

void Gui::loadFilesList(){
    fileList->clear();

    quint64 comp = 0;
    QString buffer;
    for(auto& item : currentArchive->getFiles()){
        buffer = item.getFilename();
        buffer += " (";
        if(item.getCompressedSize() == 0){
            buffer += "uncompressed yet";
        }else{
            buffer += this->locale().formattedDataSize(item.getCompressedSize());
        }
        buffer += ")";
        fileList->addItem(buffer);
        comp += item.getCompressedSize();
    }

    buffer = QString::number(currentArchive->getNumOfFiles()) + " file";
    if(currentArchive->getNumOfFiles())
        buffer += 's';

    buffer += " | Packed size: ";
    buffer += this->locale().formattedDataSize(comp);

    summaryLabel->setText(buffer);
}

bool Gui::on_saveButton_clicked(){
    if(currentArchive->isNewArchive()){
        QString fileName;
        QString defaultFileName = "archive_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + QString(Archive::archive_extension);
        fileName = QFileDialog::getSaveFileName(this,
                                                tr("Save File"),
                                                defaultFileName,
                                                tr("riv archive (*.riv)"));

        if(fileName.isEmpty()){
            return false;
        }

        QFileInfo info(fileName);
        currentArchive->setPath(info.filePath());
    }

    if(!currentArchive->save()){
        return false;
    }

    QMessageBox msgBox;
    msgBox.setText("Saved!");
    msgBox.exec();

    currentArchive->resetModified();

    resetWindow();
    ui_saveButton->setEnabled(false);
    ui_discardButton->setEnabled(false);

    return true;
}

void Gui::on_newButton_clicked(){
    resetWindow();
}

void Gui::on_discardButton_clicked(){
    resetWindow();
}

void Gui::on_addButton_clicked(){
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select file"), QDir::currentPath(), tr("All files (*)"));

    if(fileNames.empty()){
        return;
    }

    quint64 added = 0;
    for(const QString& s : fileNames){
        added += (int)addFile(s);
    }

    loadFilesList();
    ui_saveButton->setEnabled(true);

    QMessageBox msgBox;
    msgBox.setText(QString::number(added) + "/" + QString::number(fileNames.size()) + " files added");
    msgBox.exec();
}

bool Gui::addFile(QString fileName){

    QFileInfo info(fileName);

    if(currentArchive->isInArchive(info.filePath())){
        QMessageBox msgBox;
        msgBox.setText("File: " + info.fileName() + " already exists in this archive!");
        msgBox.exec();
        return false;
    }

    currentArchive->addFile(fileName);

    return true;
}

void Gui::closeEvent(QCloseEvent *event){
    if (!currentArchive->isModified()){
        event->accept();
        return;
    }

    const QMessageBox::StandardButton ret =
            QMessageBox::warning(this,
                                tr("Archiver"),
                                tr("Archive has been modified.\nSave changes?"),
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(ret){
        case QMessageBox::Save:{
            if(on_saveButton_clicked()){
                event->accept();
            }else{
                event->ignore();
            }
            break;
        }
        case QMessageBox::Discard:{
            event->accept();
            break;
        }
        default:
            event->ignore();
            break;
    }
}

void Gui::on_removeButton_clicked(){
    if(!checkIfArchiveContainsFiles()){
        return;
    }

    QList<QListWidgetItem*> selected = fileList->selectedItems();

    if(selected.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("Select files first!");
        msgBox.exec();
        return;
    }

    QString buffer;
    QRegularExpression re("^(.*) \\(");
    for(QList<QListWidgetItem*>::iterator s = selected.begin(); s != selected.end(); s++){
        buffer = (*s)->text();
        QRegularExpressionMatch match = re.match(buffer);
        if(match.hasMatch()){
            buffer = match.captured(1);
            currentArchive->removeFile(buffer);
        }
    }

    loadFilesList();

    ui_saveButton->setEnabled(true);
}

void Gui::on_unpackButton_clicked(){
    if(!checkIfArchiveContainsFiles()){
        return;
    }

    QList<QListWidgetItem*> selected = fileList->selectedItems();

    if(selected.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("Select files first!");
        msgBox.exec();
        return;
    }

    if(selected.size() == currentArchive->getNumOfFiles()){
        on_unpackAllButton_clicked();
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", nullptr);

    if(dir.isEmpty()){
        return;
    }

    QString buffer;
    QRegularExpression re("^(.*) \\(");
    quint32 unpacked = 0;
    for(QList<QListWidgetItem*>::iterator s = selected.begin(); s != selected.end(); s++){
        buffer = (*s)->text();
        QRegularExpressionMatch match = re.match(buffer);
        if(match.hasMatch()){
            buffer = match.captured(1);
            if(currentArchive->unpackFile(buffer, dir)){
                unpacked++;
            }
        }
    }

    QMessageBox::information(this, "Unpack",
                             "Unpacked " + QString::number(unpacked) + '/' + QString::number(selected.size()) + " files");
}

bool Gui::checkIfArchiveContainsFiles(){
    if(currentArchive->isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("There are no files!");
        msgBox.exec();
        return false;
    }
    return true;
}

void Gui::on_unpackAllButton_clicked(){
    if(!checkIfArchiveContainsFiles()){
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", nullptr);
    if(dir.isEmpty()){
        return;
    }

    quint32 unpacked = currentArchive->unpackAll(dir);

    QMessageBox::information(this, "Unpack",
                             "Unpacked " + QString::number(unpacked) + '/' + QString::number(currentArchive->getNumOfFiles()) + " files");
}

void Gui::dragEnterEvent(QDragEnterEvent *e){
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Gui::dropEvent(QDropEvent *event){
    QWidget::dropEvent(event);
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()){
        int added = 0;
        QList<QUrl> urlList = mimeData->urls();
        for(const QUrl& url : urlList){
            if(addFile(url.toLocalFile())){
                added++;
            }
        }

        loadFilesList();
        ui_saveButton->setEnabled(true);

        QMessageBox msgBox;
        msgBox.setText("Added " + QString::number(added) + "/" + QString::number(urlList.size())+ " files");
        msgBox.exec();
    }
}
