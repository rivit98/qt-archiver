#include "gui.h"
#include "ui_gui.h"
#include "archive.h"
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>


Gui::Gui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Gui)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    curArchive = nullptr;

    ui_openbutton = findChild<QPushButton*>("openButton");
    ui_saveButton = findChild<QPushButton*>("saveButton");
    ui_discardButton = findChild<QPushButton*>("discardButton");
    ui_newButton = findChild<QPushButton*>("newButton");
    fileList = findChild<QListWidget*>("listFiles");
    box = findChild<QGroupBox*>("groupBox");
    actionBox = findChild<QGroupBox*>("actionBox");
    actionBox->setEnabled(false);
    inputName = findChild<QLineEdit*>("nameTextArea");
    summaryLabel = findChild<QLabel*>("summaryLabel");
}

Gui::~Gui()
{
    delete ui;
    if(curArchive != nullptr){
        delete curArchive;
    }
}

void Gui::on_openButton_clicked()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("riv archive (*.riv)"));

    if(fileName.size() == 0){
        return;
    }
    ui_openbutton->setEnabled(false);
    ui_discardButton->setEnabled(true);
    box->setEnabled(false);
    actionBox->setEnabled(true);

    curArchive = new Archive(fileName.toStdString());

    QFileInfo fileInfo(fileName);
    QString title(fileInfo.fileName());
    unsigned int numOfFiles = curArchive->getNumOfFiles();
    ui_openbutton->setText(title + " | " + QString::number(numOfFiles) + " file" + ((numOfFiles == 1) ? "" : "s"));

    loadFilesList();
}

void Gui::loadFilesList(){
    fileList->clear();
    auto v = curArchive->getFiles();
    quint64 comp = 0;
    quint64 uncomp = 0;
    quint64 temp = 0;
    QString buffer;
    for(auto& item : v){
        buffer = QString::number(fileList->count()+1);
        buffer += ") ";
        temp = item.getDataLength();
        comp += temp;
        buffer += this->locale().formattedDataSize(temp);
        buffer += " (";
        temp = item.getRealDataLen();
        uncomp += temp;
        buffer += this->locale().formattedDataSize(temp);
        buffer += ") | ";
        buffer += QString::fromStdString(item.getFilename());
        fileList->addItem(buffer);
    }

    buffer = QString::number(v.size()) + " file";
    if(v.size())
        buffer += 's';
    buffer += " | Real size: ";
    buffer += this->locale().formattedDataSize(uncomp) + " (";
    buffer += this->locale().formattedDataSize(comp) + " packed) | Saved: ";
    buffer += this->locale().formattedDataSize(uncomp-comp);

    summaryLabel->setText(buffer);
}

void Gui::on_addButton_clicked(){
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select file"), QDir::currentPath(), tr("All files (*)"));

    if(fileNames.size() == 0){
        return;
    }

    for(QString& s : fileNames){
        addFile(s);
    }
}

void Gui::on_newButton_clicked(){
    resetWindow();
}
void Gui::on_discardButton_clicked(){
    resetWindow();
}
void Gui::on_saveButton_clicked(){
    if(curArchive == nullptr){
        return;
    }

    if(curArchive->hasPath()){
        curArchive->save();
    }else{
        QString fileName;
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), inputName->text(), tr("riv archive (*.riv)"));

        if(fileName.size() == 0){
            return;
        }

        QFileInfo info(fileName);
        inputName->setText(info.baseName());
        curArchive->setPath(fileName.toStdString());
        curArchive->save();
    }

    ui_saveButton->setText("SAVED");
    ui_saveButton->setEnabled(false);
    ui_discardButton->setEnabled(false);
    ui_newButton->setEnabled(true);
    actionBox->setEnabled(false);
    box->setEnabled(false);
}

void Gui::closeEvent(QCloseEvent *event){
    if(curArchive == nullptr){
        event->accept();
        return;
    }
    if (!curArchive->isModified()){
        event->accept();
        return;
    }

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Archiver"),
                               tr("The archive has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch(ret){
    case QMessageBox::Save:{
        curArchive->save();
        event->accept();
        break;
    }
    case QMessageBox::Discard:{
        event->accept();
        break;
    }
    case QMessageBox::Cancel:{
        event->ignore();
        break;
    }
    default:
        event->ignore();
        break;
    }
}

void Gui::on_nameTextArea_textChanged(){
    bool show = (bool)inputName->text().size();
    ui_openbutton->setEnabled(!show);
    actionBox->setEnabled(show);
    ui_discardButton->setEnabled(show);
}

void Gui::on_removeButton_clicked(){
    QList<QListWidgetItem*> selected = fileList->selectedItems();

    if(selected.size() == 0){
        QMessageBox msgBox;
        msgBox.setText("Select files first!");
        msgBox.exec();
        return;
    }

    QString buffer;

    for(QList<QListWidgetItem*>::iterator s = selected.begin(); s != selected.end(); s++){
        buffer.clear();
        buffer = (*s)->text();
        int pos = buffer.indexOf('|');
        buffer = buffer.mid(pos + 2); // characer + space

        curArchive->remove(buffer.toStdString());
    }

    loadFilesList();

    ui_saveButton->setEnabled(true);
}

void Gui::on_unpackButton_clicked(){
    QList<QListWidgetItem*> selected = fileList->selectedItems();

    if(selected.size() == 0){
        QMessageBox msgBox;
        msgBox.setText("Select files first!");
        msgBox.exec();
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", nullptr);

    if(dir.size() == 0){
        return;
    }

    QString buffer;
    int unpacked = 0;
    for(QList<QListWidgetItem*>::iterator s = selected.begin(); s != selected.end(); s++){
        buffer.clear();
        buffer = (*s)->text();
        int pos = buffer.indexOf('|');
        buffer = buffer.mid(pos + 2); // characer + space

        if(curArchive->unpack(buffer.toStdString(), dir.toStdString())){
            unpacked++;
        }
    }

    QMessageBox::information(this, "Unpack", "Unpacked " + QString::number(unpacked) + '/' + QString::number(selected.size()) + " files");
}

void Gui::on_unpackAllButton_clicked(){
    if(curArchive == nullptr || curArchive->getNumOfFiles() == 0){
        QMessageBox msgBox;
        msgBox.setText("There is no files!");
        msgBox.exec();
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", nullptr);
    if(dir.size() == 0){
        return;
    }

    int unpacked = curArchive->unpackAll(dir.toStdString());

    QMessageBox::information(this, "Unpack", "Unpacked " + QString::number(unpacked) + '/' + QString::number(curArchive->getNumOfFiles()) + " files");
}

void Gui::resetWindow(){
    summaryLabel->setText("0 files | Real size: 0 (0 packed) | Saved: 0");
    ui_openbutton->setEnabled(true);
    ui_openbutton->setText("Open archive");
    ui_saveButton->setText("Save archive");
    ui_saveButton->setEnabled(false);
    ui_discardButton->setEnabled(false);
    actionBox->setEnabled(false);
    ui_newButton->setEnabled(false);
    box->setEnabled(true);
    fileList->clear();
    inputName->clear();

    if(curArchive != nullptr)
        delete curArchive;
    curArchive = nullptr;
}

bool Gui::addFile(QString fileName){

    QFileInfo info(fileName);

    if(info.size() > 52428800){
        QMessageBox msgBox;
        msgBox.setText("Sorry i don't support files bigger than 50MB :(");
        msgBox.exec();
        return false;
    }

    if(curArchive != nullptr){
        if(curArchive->isInArchive(info.fileName().toStdString(), info.size())){
            QMessageBox msgBox;
            msgBox.setText("File: [" + info.fileName() + "] already exists in this archive!");
            msgBox.exec();
            return false;
        }

        if((curArchive->getPath() == info.fileName().toStdString() || curArchive->getPath() == info.filePath().toStdString()) && info.size() == curArchive->size()){
            QMessageBox msgBox;
            msgBox.setText("You can't add archive that you are currently working on!");
            msgBox.exec();
            return false;
        }

    }else{
        curArchive = new Archive;
    }

    curArchive->add(fileName.toStdString());

    loadFilesList();
    ui_saveButton->setEnabled(true);

    return true;
}

void Gui::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Gui::dropEvent(QDropEvent * event){
    QWidget::dropEvent(event);
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        int added = 0;
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size(); ++i)
        {
            if(addFile(urlList.at(i).toLocalFile())){
                added++;
            }
        }

        QMessageBox msgBox;
        msgBox.setText("Added " + QString::number(added) + "/" + QString::number(urlList.size())+ " files");
        msgBox.exec();
    }
}
