/***************************************************************************
 *   Copyright (C) 2018 by AndreyBarmaley  <public.irkutsk@gmail.com>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QFileDialog>
#include <QSettings>
#include <QMenu>
#include <QProcess>
#include <QAction>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QStringList>

#include "mainwidget.h"
#include "ui_mainwidget.h"

static int build = 20180427;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget), version("1.4")
{
    QIcon icon = QIcon(":/images/watchfolder.png");

    // actions
    actionRestoreWindow = new QAction(tr("&Restore"), this);
    actionQuitApp = new QAction(tr("&Quit"), this);

    // tray icon
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(actionRestoreWindow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(actionQuitApp);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(icon);
    trayIcon->show();

    // watch dir
    watchDir = NULL;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "IDC", "WatchFolder");

    // ui
    ui->setupUi(this);
    ui->textEditLogs->setLineWrapMode(QTextEdit::NoWrap);
    ui->lineEditFolder->setText(settings.value("folder", "").toString());
    ui->lineEditScript->setText(settings.value("script", "").toString());

    setWindowTitle(tr("Watch Folder, version: %1 (%2)").arg(version).arg(build));
    setWindowIcon(icon);
    setFixedSize(600, 300);


    if(settings.value("lock", false).toBool())
	addLogString("settings lock: on");
    else
	addLogString("settings lock: off");
    if(settings.value("mode", "").toString() == "file")
	addLogString("settings mode: file");
    else
	addLogString("settings mode: directory");

    addLogString(QString("watchfolder version: %1 (%2)").arg(version).arg(build));

    connect(actionRestoreWindow, &QAction::triggered, this, & QWidget::showNormal);
    connect(actionQuitApp, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(trayIcon, & QSystemTrayIcon::activated, this, & MainWidget::iconActivated);

    updateButtons();
}

void MainWidget::updateButtons(void)
{
    // other
    QFileInfo folder(ui->lineEditFolder->text());
    QFileInfo script(ui->lineEditScript->text());

    if(folder.isDir() && folder.isReadable())
    {
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "IDC", "WatchFolder");

	settings.setValue("folder", ui->lineEditFolder->text());
	settings.setValue("script", ui->lineEditScript->text());

        addLogString("watching folder: " + folder.filePath());

        if(script.isFile() && script.isExecutable())
	{
    	    on_pushButtonStart_clicked();
    	    showMinimized();
	}
	else
	{
	    ui->pushButtonStart->setEnabled(false);
	    ui->pushButtonStop->setEnabled(false);
	    addLogString("error execute script: " + script.filePath());
	}
    }
    else
    {
	ui->pushButtonStart->setEnabled(false);
	ui->pushButtonStop->setEnabled(false);
	addLogString("error read directory: " + folder.filePath());
    }
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::closeEvent(QCloseEvent* event)
{
    if(trayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
}

void MainWidget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger:
            if(isVisible())
                hide();
            else
                showNormal();
        break;

        default:
        break;
    }
}

void MainWidget::on_pushButtonStart_clicked()
{
    ui->lineEditFolder->setEnabled(false);
    ui->lineEditScript->setEnabled(false);
    ui->lineEditParams->setEnabled(false);
    ui->frameParams->setEnabled(false);
    ui->toolButtonFolder->setEnabled(false);
    ui->toolButtonScript->setEnabled(false);
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(true);

    const QString & folder = ui->lineEditFolder->text();
    const QString & script = ui->lineEditScript->text();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "IDC", "WatchFolder");
    settings.setValue("folder", folder);
    settings.setValue("script", script);

    if(settings.value("lock", false).toBool())
    {
	ui->pushButtonStop->setEnabled(false);
	actionQuitApp->setEnabled(false);
    }

    QFileInfo folderInfo(folder);
    QFileInfo scriptInfo(script);

    files = QSet<QString>::fromList(QDir(folder).entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable));

    if(folderInfo.isDir() && folderInfo.isReadable() &&
            scriptInfo.isFile() && scriptInfo.isExecutable())
    {
        if(watchDir) delete watchDir;

        watchDir = new QFileSystemWatcher(this);
        watchDir->addPath(folder);

	if(settings.value("mode", "").toString() == "file")
	{
	    connect(watchDir, &QFileSystemWatcher::fileChanged, this, & MainWidget::executeScript);
	}
	else
        {
	    connect(watchDir, &QFileSystemWatcher::directoryChanged, this, & MainWidget::executeScript);
	}

        addLogString("watching starting...");
        addLogString("current files: " + QString::number(files.size()));
    }
}

void MainWidget::on_pushButtonStop_clicked()
{
    ui->lineEditFolder->setEnabled(true);
    ui->lineEditScript->setEnabled(true);
    ui->lineEditParams->setEnabled(true);
    ui->frameParams->setEnabled(true);
    ui->toolButtonFolder->setEnabled(true);
    ui->toolButtonScript->setEnabled(true);
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);

    if(watchDir) delete watchDir;
    watchDir = NULL;

    addLogString("stop watching");
}

void MainWidget::executeScript(const QString & dir)
{
    const QString & params = ui->lineEditParams->text();
    const QString & script = ui->lineEditScript->text();

    QFileInfo scriptInfo(script);
    QDir dirInfo(dir);

    QStringList addFiles, delFiles;

    if(true)
    {
        const QSet<QString> & newFiles = QSet<QString>::fromList(QDir(dir).entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable));
        const QSet<QString> & subFiles = newFiles - files;

        foreach(auto it, subFiles)
        {
            const QString & file = dirInfo.absoluteFilePath(it);

            if(QFileInfo(file).exists())
            addFiles << file;
            else
            delFiles << file;
        }

        files = newFiles;
    }

    if(scriptInfo.isFile() && scriptInfo.isExecutable())
    {
        foreach(auto it, addFiles)
        {
                addLogString("add file: " + it);
                addLogString("run script: " + script);

                QProcess process;
                process.start(script, QStringList() << dir);
                process.waitForFinished(-1);
        }

        if(delFiles.size())
                addLogString("del files: " + QString::number(delFiles.size()));
    }
}

void MainWidget::addLogString(const QString & str)
{
    ui->textEditLogs->insertPlainText(QString("[%1]\t%2\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(str));
    ui->pushButtonClear->setEnabled(true);
}

void MainWidget::on_toolButtonFolder_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "",
                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(! dir.isEmpty())
        ui->lineEditFolder->setText(dir);

    updateButtons();
}

void MainWidget::on_toolButtonScript_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", "*.*");

    if(! file.isEmpty())
        ui->lineEditScript->setText(file);

    updateButtons();
}

void MainWidget::on_pushButtonClear_clicked()
{
    ui->textEditLogs->clear();
    ui->pushButtonClear->setEnabled(false);
    addLogString("clear logs");
}
