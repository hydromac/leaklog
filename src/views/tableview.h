/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "inspectiondetailsview.h"
#include "defs.h"

class HTMLTableCell;

class TableView : public InspectionDetailsView
{
    Q_OBJECT

public:
    TableView(ViewTabSettings *settings);

    QString renderHTML();

    QString title() const;

protected:
    HTMLTable *writeInspectionsTable(const QVariantMap &, const QVariantMap &, ListOfVariantMaps &, VariableEvaluation::EvaluationContext &);
    QStringList listDelayedWarnings(Warnings &, const QVariantMap &, QVariantMap &, const QString &, const QString &, int * = NULL);
    void writeTableVarCell(MTTextStream &, const QString &, const QString &, const QString &, const QString &, bool, int, double);
    HTMLTableCell *writeTableVarCell(const QString &, const QString &, const QString &, const QString &, bool, int, double);
    QString tableVarValue(const QString &, const QString &, const QString &, const QString &, bool, double, bool = false);
};

#endif // TABLEVIEW_H
