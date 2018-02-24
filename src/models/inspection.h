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

#ifndef INSPECTION_H
#define INSPECTION_H

#include "dbrecord.h"

class MDComboBox;
class Customer;
class Circuit;
class InspectionCompressor;
class InspectionFile;

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    enum Repair {
        IsNotRepair = 0,
        IsRepair = 1,
        IsAfterRepair = 2,
    };
    enum Type {
        DefaultType = 0,
        CircuitMovedType = 1,
        InspectionSkippedType = 2
    };

    inline bool nominal() { return value("nominal").toInt() != 0; }
    inline Repair repair() { return (Repair)value("repair").toInt(); }
    inline Type inspectionType() { return (Type)value("inspection_type").toInt(); }

    static QString titleForInspection(bool nominal, Repair repair);
    static QString descriptionForInspectionType(Type type, const QString &type_data);

    static QString tableName();
    static inline MTRecordQuery<Inspection> query(const MTDictionary &parents = MTDictionary()) { return MTRecordQuery<Inspection>(tableName(), parents); }
    static MTQuery queryByInspector(const QString &inspector_uuid);
    static const ColumnList &columns();

    Inspection(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(QWidget * = 0);

    int scope() { return m_scope; }

    inline QString customerUUID() { return stringValue("customer_uuid"); }
    inline void setCustomerUUID(const QString &value) { setValue("customer_uuid", value); }
    Customer customer();
    inline QString circuitUUID() { return stringValue("circuit_uuid"); }
    inline void setCircuitUUID(const QString &value) { setValue("circuit_uuid", value); }
    Circuit circuit();
    inline QString date() { return stringValue("date"); }
    inline void setDate(const QString &value) { setValue("date", value); }
    inline bool isNominal() { return intValue("nominal"); }
    inline void setNominal(bool value) { setValue("nominal", (int)value); }
    inline bool isRepair() { return intValue("repair") == IsRepair; }
    inline void setRepair(bool value) { setValue("repair", (int)value); }
    inline bool isOutsideInterval() { return intValue("outside_interval"); }
    inline void setOutsideInterval(bool value) { setValue("outside_interval", (int)value); }
    inline Type type() { return (Type)intValue("inspection_type"); }
    inline void setType(Type value) { setValue("inspection_type", (int)value); }
    inline QString typeData() { return stringValue("inspection_type_data"); }
    inline void setTypeData(const QString &value) { setValue("inspection_type_data", value); }

    MTRecordQuery<InspectionCompressor> compressors() const;
    MTRecordQuery<InspectionFile> files() const;

    bool remove() const;

public slots:
    void showSecondNominalInspectionWarning(MDComboBox *, int);

private:
    int m_scope;
};

#endif // INSPECTION_H
