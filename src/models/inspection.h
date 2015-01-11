/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef INSPECTION_H
#define INSPECTION_H

#include "dbrecord.h"

class MTCheckBox;

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    enum Type {
        DefaultType = 0,
        CircuitMovedType = 1,
        InspectionSkippedType = 2
    };

    static QString descriptionForInspectionType(Type type, const QString &type_data);

    static QString tableName();
    static const ColumnList &columns();

    Inspection();
    Inspection(const QString &, const QString &, const QString &);
    Inspection(const QString &, const QString &, const QString &, const MTDictionary &);

    void initEditDialogue(EditDialogueWidgets *);

    int scope() { return m_scope; }

public slots:
    void showSecondNominalInspectionWarning(MTCheckBox *, bool);

private:
    int m_scope;
};

class InspectionByInspector : public Inspection
{
    Q_OBJECT

public:
    InspectionByInspector(const QString &);
};

#endif // INSPECTION_H
