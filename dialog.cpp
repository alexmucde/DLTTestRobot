/**
 * @licence app begin@
 * Copyright (C) 2021 Alexander Wenzel
 *
 * This file is part of the DLT Relais project.
 *
 * \copyright This code is licensed under GPLv3.
 *
 * \author Alexander Wenzel <alex@eli2.de>
 *
 * \file dialog.cpp
 * @licence end@
 */

#include <QSerialPortInfo>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDir>

#include "dialog.h"
#include "ui_dialog.h"
#include "settingsdialog.h"
#include "version.h"

Dialog::Dialog(bool autostart,QString configuration,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , dltTestRobot(this)
{
    ui->setupUi(this);

    // clear settings
    on_pushButtonDefaultSettings_clicked();

    // set window title with version information
    setWindowTitle(QString("DLTTestRobot %1").arg(DLT_TEST_ROBOT_VERSION));
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    // disable stop button at startup
    ui->pushButtonStop->setDisabled(true);

    // connect status slots
    connect(&dltTestRobot, SIGNAL(status(QString)), this, SLOT(statusTestRobot(QString)));
    connect(&dltTestRobot, SIGNAL(statusTests(QString)), this, SLOT(statusTests(QString)));
    connect(&dltTestRobot, SIGNAL(report(QString)), this, SLOT(report(QString)));
    connect(&dltTestRobot, SIGNAL(reportSummary(QString)), this, SLOT(reportSummary(QString)));
    connect(&dltMiniServer, SIGNAL(status(QString)), this, SLOT(statusDlt(QString)));

    connect(&dltTestRobot, SIGNAL(command(int,int,int,int,int,int,int,QString)), this, SLOT(command(int,int,int,int,int,int,int,QString)));

    //  load global settings from registry
    QSettings settings;
    QString filename = settings.value("autoload/filename").toString();
    bool autoload = settings.value("autoload/checked").toBool();
    bool autostartGlobal = settings.value("autostart/checked").toBool();
    bool autoloadTests = settings.value("autoloadTests/checked").toBool();
    QString filenameTests = settings.value("autoloadTests/filename").toString();

    // autoload settings, when activated in global settings
    if(autoload)
    {
        dltTestRobot.readSettings(filename);
        dltMiniServer.readSettings(filename);
        restoreSettings();
    }

    // autoload tests, when activated in global settings
    if(autoloadTests)
    {
        ui->checkBoxAutoloadTests->setChecked(true);
        loadTests(filenameTests);
    }

    // autoload settings, when provided by command line
    if(!configuration.isEmpty())
    {
        dltTestRobot.readSettings(configuration);
        dltMiniServer.readSettings(configuration);
        restoreSettings();
    }

    ui->pushButtonStartTest->setEnabled(false);

    // autostart, when activated in global settings or by command line
    if(autostartGlobal || autostart)
    {
        on_pushButtonStart_clicked();
    }

    ui->pushButtonStopTest->setEnabled(false);
}

Dialog::~Dialog()
{

    // disconnect all slots
    disconnect(&dltTestRobot, SIGNAL(status(QString)), this, SLOT(statusTestRobot(QString)));
    disconnect(&dltMiniServer, SIGNAL(status(QString)), this, SLOT(statusDlt(QString)));

    delete ui;
}

void Dialog::restoreSettings()
{
}

void Dialog::updateSettings()
{
}

void Dialog::on_pushButtonStart_clicked()
{
    // start communication
    updateSettings();

    // start Relais and DLT communication
    dltTestRobot.start();
    dltMiniServer.start();

    // disable settings and start button
    // enable stop button
    ui->pushButtonStart->setDisabled(true);
    ui->pushButtonStop->setDisabled(false);
    ui->pushButtonDefaultSettings->setDisabled(true);
    ui->pushButtonLoadSettings->setDisabled(true);
    ui->pushButtonSettings->setDisabled(true);
    ui->pushButtonStartTest->setEnabled(true);
    ui->pushButtonStopTest->setEnabled(false);

    //connect(&dltTestRobot, SIGNAL(message(unsigned int,QByteArray)), this, SLOT(message(unsigned int,QByteArray)));
}

void Dialog::on_pushButtonStop_clicked()
{
    // stop communication

    //disconnect(&dltTestRobot, SIGNAL(message(unsigned int,QByteArray)), this, SLOT(message(unsigned int,QByteArray)));

    // stop Relais and DLT communication
    dltTestRobot.stop();
    dltMiniServer.stop();

    // enable settings and start button
    // disable stop button
    ui->pushButtonStart->setDisabled(false);
    ui->pushButtonStop->setDisabled(true);
    ui->pushButtonDefaultSettings->setDisabled(false);
    ui->pushButtonLoadSettings->setDisabled(false);
    ui->pushButtonSettings->setDisabled(false);
    ui->pushButtonStartTest->setEnabled(false);
    ui->pushButtonStopTest->setEnabled(false);

}

void Dialog::statusTestRobot(QString text)
{
    // status from Test Robot

    // status of Test Robot communication changed
    if(text == "" || text == "stopped" || text == "not active")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "connected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "connect" || text == "disconnected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "error")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
}

void Dialog::statusTests(QString text)
{
    // status from Tests

    if(text == "Prerun")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
    }
    else if(text == "Running")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
    }
    else if(text == "Postrun")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
    }
    else if(text == "Finished")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);

        // close DLT file and disconnect ECUs
        dltTestRobot.send(QString("disconnectAllEcu"));
        dltTestRobot.send(QString("clearFile"));

        // update UI
        ui->pushButtonStartTest->setEnabled(true);
        ui->pushButtonStopTest->setEnabled(false);
        ui->pushButtonTestLoad->setEnabled(true);
        ui->comboBoxTestName->setEnabled(true);
        ui->checkBoxRunAllTest->setEnabled(true);
        ui->lineEditRepeat->setEnabled(true);
        ui->pushButtonStop->setEnabled(true);
    }
}

void Dialog::statusDlt(QString text)
{
    // status from DLT Mini Server
    ui->lineEditStatusDLT->setText(text);

    // status of DLT communication changed
    if(text == "" || text == "stopped")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "listening")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "connected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "error")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditStatusDLT->setPalette(palette);
    }
}

void Dialog::report(QString text)
{
    // write to report
    QTime time = QTime::currentTime();
    reportFile.write(QString("=> %1 %2\n").arg(time.toString("HH:mm:ss")).arg(text).toLatin1());
}

void Dialog::reportSummary(QString text)
{
    // write to report
    reportSummaryList.append(text);
}

void Dialog::on_pushButtonDefaultSettings_clicked()
{
    // Reset settings to default
    dltTestRobot.clearSettings();
    dltMiniServer.clearSettings();

    restoreSettings();
}

void Dialog::on_pushButtonLoadSettings_clicked()
{
    // Load settings from XML file

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Settings"), "", tr("DLTTestRobot Settings (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    // read the settings from XML file
    dltTestRobot.readSettings(fileName);
    dltMiniServer.readSettings(fileName);

    restoreSettings();
}

void Dialog::on_pushButtonSaveSettings_clicked()
{
    // Save settings into XML file

    updateSettings();

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Settings"), "", tr("DLTTestRobot Settings (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    // read the settings from XML file
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        // Cannot open the file for writing
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);

    // FIXME: Cannot read data from XML file, which contains a start document
    // So currently do not call StartDocument
    //xml.writeStartDocument();

    xml.writeStartElement("DLTCanSettings");
        dltTestRobot.writeSettings(xml);
        dltMiniServer.writeSettings(xml);
    xml.writeEndElement(); // DLTRelaisSettings

    // FIXME: Cannot read data from XML file, which contains a end document
    // So currently do not call EndDocument
    //xml.writeEndDocument();
    file.close();

}

void Dialog::on_pushButtonSettings_clicked()
{
    // Open settings dialog
    SettingsDialog dlg(this);

    dlg.restoreSettings(&dltTestRobot, &dltMiniServer);
    if(dlg.exec()==QDialog::Accepted)
    {
        dlg.backupSettings(&dltTestRobot, &dltMiniServer);
        restoreSettings();
    }
}

void Dialog::on_pushButtonInfo_clicked()
{
    // Open information window
    QMessageBox msgBox(this);

    msgBox.setWindowTitle("Info DLTPower");
    msgBox.setTextFormat(Qt::RichText);

    QString text;
    text += QString("Version: %1<br>").arg(DLT_TEST_ROBOT_VERSION);
    text += "<br>";
    text += "Information and Documentation can be found here:<br>";
    text += "<br>";
    text += "<a href='https://github.com/alexmucde/DLTCan'>Github DLTTestRobot</a><br>";
    text += "<br>";
    text += "This SW is licensed under GPLv3.<br>";
    text += "<br>";
    text += "(C) 2022 Alexander Wenzel <alex@eli2.de>";

    msgBox.setText(text);

    msgBox.setStandardButtons(QMessageBox::Ok);

    msgBox.exec();
}

void Dialog::on_pushButtonTestLoad_clicked()
{
    // Load test file

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Tests"), "", tr("DLTTestRobot Tests (*.dtr);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    QSettings settings;
    settings.setValue("autoloadTests/filename",fileName);

    loadTests(fileName);
}

void Dialog::loadTests(QString fileName)
{
    ui->lineEditTestFile->setText(fileName);

    // read the tests from dtr file
    QStringList errors = dltTestRobot.readTests(fileName);

    if(!errors.isEmpty())
    {
        // There are some error during parsing test file
        QMessageBox msgBox(QMessageBox::Critical,"Loading Test Cases","");
        msgBox.setText(("Loading all Test Cases failed with following Errors:\n\n")+errors.join('\n')+("\n\nPlease fix Errors and Reload Test Cases!"));
        msgBox.exec();
    }
    else
    {
        // Loading Test Cases were succesful
        QMessageBox msgBox;
        msgBox.setText(QString("Loading all Test Cases was succesful:\n\nNumber of Test Cases: %1").arg(dltTestRobot.size()));
        msgBox.exec();
    }

    ui->comboBoxTestName->clear();
    for(int num=0;num<dltTestRobot.size();num++)
    {
        ui->comboBoxTestName->addItem(QString("%1 %2 (%3)").arg(dltTestRobot.testId(num)).arg(dltTestRobot.testSize(num)).arg(dltTestRobot.testDescription(num)));
    }
}

void Dialog::on_pushButtonStartTest_clicked()
{
    startTests();
}

void Dialog::on_pushButtonStopTest_clicked()
{
    dltTestRobot.stopTest();

    dltMiniServer.sendValue("test stopped ",DLT_LOG_FATAL);
}

void Dialog::startTests()
{
    // update Command Number
    ui->lineEditCmdNo->setText(QString("%1/%2").arg(0).arg(dltTestRobot.testSize(ui->comboBoxTestName->currentIndex())));

    // create and write to report
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    if(reportFile.isOpen())
        reportFile.close();
    reportSummaryList.clear();
    reportFailedCounter=0;
    reportSuccessCounter=0;
    if(ui->checkBoxRunAllTest->isChecked())
        reportFile.setFileName(date.toString("reports\\yyyyMMdd_")+time.toString("HHmmss_")+QFileInfo(dltTestRobot.getTestsFilename()).baseName()+"_TestReport.txt");
    else
        reportFile.setFileName(date.toString("reports\\yyyyMMdd_")+time.toString("HHmmss_")+dltTestRobot.testId(ui->comboBoxTestName->currentIndex())+"_TestReport.txt");
    reportFile.open(QIODevice::WriteOnly | QIODevice::Text);
    reportFile.write(QString("Starting tests at %1 %2\n").arg(date.toString("dd.MM.yyyy")).arg(time.toString("HH:mm:ss")).toLatin1());
    reportFile.write(QString("\nTest File: %1\n").arg(dltTestRobot.getTestsFilename()).toLatin1());
    reportFile.write(QString("\nTest Version: %1\n\n").arg(dltTestRobot.getVersion()).toLatin1());
    reportFile.flush();

    // open new DLt file and connect all ECUs
    if(ui->checkBoxRunAllTest->isChecked())
        dltTestRobot.send(QString("newFile ")+QDir::currentPath()+date.toString("\\reports\\yyyyMMdd_")+time.toString("HHmmss_")+QFileInfo(dltTestRobot.getTestsFilename()).baseName()+"_Logs.dlt");
    else
        dltTestRobot.send(QString("newFile ")+QDir::currentPath()+date.toString("\\reports\\yyyyMMdd_")+time.toString("HHmmss_")+dltTestRobot.testId(ui->comboBoxTestName->currentIndex())+"_Logs.dlt");
    dltTestRobot.send(QString("connectAllEcu"));

    // update UI
    ui->pushButtonStartTest->setEnabled(false);
    ui->pushButtonStopTest->setEnabled(true);
    ui->pushButtonTestLoad->setEnabled(false);
    ui->comboBoxTestName->setEnabled(false);
    ui->checkBoxRunAllTest->setEnabled(false);
    ui->lineEditRepeat->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);

    // start the tests and write info to log
    dltMiniServer.sendValue2("Tests start",QFileInfo(dltTestRobot.getTestsFilename()).baseName());
    if(ui->checkBoxRunAllTest->isChecked())
        dltTestRobot.startTest(-1,ui->lineEditRepeat->text().toInt());
    else
        dltTestRobot.startTest(ui->comboBoxTestName->currentIndex(),ui->lineEditRepeat->text().toInt());
}

void Dialog::stopTests()
{
    // write summary and close report
    writeSummaryToReport();
    reportFile.close();
}

void Dialog::writeSummaryToReport()
{
    // write summary
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    reportFile.write(QString("\nSummary\n").toLatin1());
    for(int num=0;num<reportSummaryList.size();num++)
    {
        reportFile.write((reportSummaryList[num]+"\n").toLatin1());
    }
    reportFile.write(QString("\nSuccess tests: %1\nFailed tests: %2\nFailed test commands: %3\n").arg(reportSuccessCounter).arg(reportFailedCounter).arg(dltTestRobot.getFailedTestCommands()).toLatin1());
    reportFile.write(QString("\nTests STOPPED at %1 %2\n").arg(date.toString("dd.MM.yyyy")).arg(time.toString("HH:mm:ss")).toLatin1());
}

void Dialog::command(int allTestRepeatNum,int allTestRepeat, int testRepeatNum,int testRepeat,int testNum, int commandNum,int commandCount, QString text)
{
    ui->lineEditCmdNo->setText(QString("%1/%2").arg(commandNum+1).arg(commandCount));
    ui->lineEditRepeatNo->setText(QString("%1/%2").arg(allTestRepeatNum+1).arg(allTestRepeat));
    ui->lineEditTestRepeatNo->setText(QString("%1/%2").arg(testRepeatNum+1).arg(testRepeat));
    ui->lineEditCurrentTest->setText(QString("%1 (%2)").arg(dltTestRobot.testId(testNum)).arg(dltTestRobot.testDescription(testNum)));
    ui->lineEditFailed->setText(QString("%1").arg(dltTestRobot.getFailedTestCommands()));

    if(dltTestRobot.getFailedTestCommands()>0)
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditFailed->setPalette(palette);
    }
    else
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditFailed->setPalette(palette);
    }

    if(text=="started")
    {
        dltMiniServer.sendValue2("test start",dltTestRobot.testId(testNum));
        ui->listWidgetCommands->clear();
        ui->listWidgetCommands->addItems(dltTestRobot.getTest(testNum).getCommands());
        ui->listWidgetCommands->setCurrentRow(commandNum);

        // write to report
        QTime time = QTime::currentTime();
        reportFile.write(QString("\n%1 test start %2 (%3/%4)\n").arg(time.toString("HH:mm:ss")).arg(dltTestRobot.testId(testNum)).arg(testRepeatNum+1).arg(testRepeat).toLatin1());
        reportFile.flush();
    }
    else if(text=="end success")
    {
        dltMiniServer.sendValue2("test end success",dltTestRobot.testId(testNum));

        // write to report
        QTime time = QTime::currentTime();
        reportFile.write(QString("%1 test end SUCCESS\n").arg(time.toString("HH:mm:ss")).toLatin1());
        reportFile.flush();

        reportSummaryList.append(QString("SUCCESS %1 (%2/%3)").arg(dltTestRobot.testId(testNum)).arg(testRepeatNum+1).arg(testRepeat));
        reportSuccessCounter++;
    }
    else if(text=="end")
    {
        dltMiniServer.sendValue2("Tests end",QFileInfo(dltTestRobot.getTestsFilename()).baseName());

        stopTests();

    }
    else if(text=="failed")
    {
        dltMiniServer.sendValue2("test failed",dltTestRobot.testId(testNum),DLT_LOG_FATAL);

        // write to report
        QTime time = QTime::currentTime();
        reportFile.write(QString("%1 test FAILED\n").arg(time.toString("HH:mm:ss")).toLatin1());
        reportFile.flush();

        reportSummaryList.append(QString("FAILED %1 (%2/%3)").arg(dltTestRobot.testId(testNum)).arg(testRepeatNum+1).arg(testRepeat));
        reportFailedCounter++;
        dltTestRobot.send(QString("marker"));

    }
    else if(text=="stopped")
    {
        stopTests();
    }
    else
    {
        ui->listWidgetCommands->setCurrentRow(commandNum);
        dltMiniServer.sendValue3("test step",QString("%1").arg(commandNum),text);

        // write to report
        QTime time = QTime::currentTime();
        reportFile.write(QString("%1 test step %2 %3\n").arg(time.toString("HH:mm:ss")).arg(commandNum).arg(text).toLatin1());
        reportFile.flush();
    }
}

void Dialog::on_checkBoxAutoloadTests_clicked(bool checked)
{
    // store chnaged setting in registry
    QSettings settings;
    settings.setValue("autoloadTests/checked",checked);
}

