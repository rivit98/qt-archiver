#ifndef GUI_H
#define GUI_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "archive.h"
#include <QGroupBox>
#include <QLineEdit>
#include <QCloseEvent>
#include <QLabel>


namespace Ui {
class Gui;
}

class Gui : public QWidget
{
    Q_OBJECT

public:
    explicit Gui(QWidget *parent = nullptr);
    ~Gui() override;
    void loadFilesList();
    void resetWindow();
    bool addFile(QString fileName);


private slots:
    void on_openButton_clicked();
    void on_addButton_clicked();
    void on_saveButton_clicked();
    void on_discardButton_clicked();
    void closeEvent(QCloseEvent *event) override;
    void dropEvent(QDropEvent * event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void on_nameTextArea_textChanged();
    void on_newButton_clicked();
    void on_removeButton_clicked();
    void on_unpackButton_clicked();
    void on_unpackAllButton_clicked();


private:
    Ui::Gui *ui;
    Archive* curArchive;
    QPushButton* ui_openbutton;
    QPushButton* ui_saveButton;
    QPushButton* ui_discardButton;
    QPushButton* ui_newButton;
    QListWidget* fileList;
    QGroupBox* box;
    QGroupBox* actionBox;
    QLineEdit* inputName;
    QLabel* summaryLabel;
};

#endif // GUI_H
