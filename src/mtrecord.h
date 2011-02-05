/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#ifndef MTRECORD_H
#define MTRECORD_H

#include "defs.h"
#include "mtdictionary.h"

#include <QVariant>

class QSqlQuery;

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord(): QObject() {}
    MTRecord(const QString &, const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void addFilter(const QString &, const QString &);
    inline QString table() const { return r_table; }
    inline QString idField() const { return r_id_field; }
    inline QString id() const { return r_id; }
    inline QString & id() { return r_id; }
    void setId(const QString &);
    inline MTDictionary & parents() { return r_parents; }
    inline QString parent(const QString & field) const { return r_parents.value(field); }
    bool exists();
    QSqlQuery select(const QString & = "*", Qt::SortOrder = Qt::AscendingOrder);
    QVariantMap list(const QString & = "*", bool = false);
    void readAttributes();
    inline QVariantMap & attributes() { return r_attributes; }
    inline QVariant value(const QString & field, const QVariant & default_value = QVariant()) {
        return r_attributes.isEmpty() ? list(field).value(field, default_value) : r_attributes.value(field, default_value);
    }
    inline QString stringValue(const QString & field, const QString & default_value = QString()) {
        return r_attributes.isEmpty() ? list(field).value(field, default_value).toString() : r_attributes.value(field, default_value).toString();
    }
    ListOfVariantMaps listAll(const QString & = "*");
    QVariantMap sumAll(const QString &);
    MultiMapOfVariantMaps mapAll(const QString &, const QString & = "*");
    bool update(const QVariantMap &, bool = false);
    virtual bool remove();

private:
    QString r_table;
    QString r_id_field;
    QString r_id;
    MTDictionary r_parents;
    MTDictionary r_filter;
    QVariantMap r_attributes;
};

#endif // MTRECORD_H
