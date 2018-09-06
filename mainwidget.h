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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QSet>

namespace Ui {
class MainWidget;
}

class QAction;
class QMenu;
class QFileSystemWatcher;
class QCloseEvent;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void        closeEvent(QCloseEvent*);
    void        addLogString(const QString &);
    void	updateButtons(void);

private slots:
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();
    void on_toolButtonFolder_clicked();
    void on_toolButtonScript_clicked();
    void iconActivated(QSystemTrayIcon::ActivationReason);
    void executeScript(const QString &);

    void on_pushButtonClear_clicked();

private:
    Ui::MainWidget*     ui;

    QAction*            actionRestoreWindow;
    QAction*            actionQuitApp;

    QMenu*              trayIconMenu;
    QSystemTrayIcon*    trayIcon;

    QFileSystemWatcher* watchDir;

    QSet<QString>	files;
    QString		version;
};

#endif // MAINWIDGET_H
