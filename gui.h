#ifndef GUI_H
#define GUI_H

#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMimeData>

#include "archive.h"
#include "compressor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Gui; }
QT_END_NAMESPACE

class Gui : public QWidget
{
    Q_OBJECT

private:
    Ui::Gui *ui;
    QPushButton* ui_openbutton;
    QPushButton* ui_saveButton;
    QPushButton* ui_discardButton;
    QPushButton* ui_newButton;
    QListWidget* fileList;
    QGroupBox* box;
    QGroupBox* actionBox;
    QLineEdit* inputName;
    QLabel* summaryLabel;

    QSharedPointer<Archive> currentArchive;

private:
    void loadFilesList();
    void resetWindow();


private slots:
    void closeEvent(QCloseEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void on_openButton_clicked();
    void on_nameTextArea_textChanged();
    bool on_saveButton_clicked();
    void on_newButton_clicked();
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_unpackButton_clicked();
    void on_unpackAllButton_clicked();
    void on_discardButton_clicked();


public:
    Gui(QWidget *parent = nullptr);
    ~Gui() override;
    bool addFile(QString fileName);


};
#endif // GUI_H
