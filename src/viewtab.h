/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#ifndef VIEWTAB_H
#define VIEWTAB_H

#include "viewtabsettings.h"
#include "view.h"
#include "mtwidget.h"
#include "linkparser.h"

class ToolBarStack;
class QTreeWidgetItem;
class QWebView;
class QUrl;

namespace Ui {
class ViewTab;
}

class ViewTab : public MTWidget, public ViewTabSettings
{
    Q_OBJECT

public:
    explicit ViewTab(QWidget * parent = 0);
    ~ViewTab();
    QObject * object() { return this; }

    void enableTools();

    MainWindowSettings & mainWindowSettings();

    ToolBarStack * toolBarStack() const;
    QWebView * webView() const;

    View * view(View::ViewID view) { return views[view]; }
    View::ViewID currentView() const;
    QString currentTable() const;

    bool isShowDateUpdatedChecked() const;
    bool isShowOwnerChecked() const;
    bool isCompareValuesChecked() const;
    bool isPrinterFriendlyVersionChecked() const;

    QString appendDefaultOrderToColumn(const QString & column) const;

public slots:
    void loadPreviousLink();
    void loadNextLink();

    void setView(View::ViewID view);
    void refreshView();

    void reportData();
    void reportDataFinished();

signals:
    void enableBackButton(bool);
    void enableForwardButton(bool);

    void viewChanged(View::ViewID);

protected:
    void emitEnableBackAndForwardButtons(bool enableBack, bool enableForward);

private slots:
    void reloadTables(const QStringList & tables);
    void addTable(int index, const QString & table);
    void removeTable(const QString & table);

    void viewChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);

    void executeLink(const QUrl &);
    void executeLink(Link * link);
    void saveLink(int);

private:
    void createViewItems();
    void formatGroupItem(QTreeWidgetItem * item);

    void setDefaultWebPage();

    Ui::ViewTab * ui;
    View * views[View::ViewCount];
    QTreeWidgetItem * group_tables;
    QTreeWidgetItem * view_items[View::ViewCount];
};

#endif // VIEWTAB_H