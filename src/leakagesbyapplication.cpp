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

#include "leakagesbyapplication.h"
#include "global.h"
#include "records.h"

#include <QDate>

using namespace Global;

const QString LeakagesByApplication::Key::All("All");

LeakagesByApplication::LeakagesByApplication(bool weighted_averages):
    QObject(), min_year(9999), max_year(0)
{
    tables << variableNames().value("refr_add_am") << tr("Amount of refrigerant in circuits") << tr("Percentage of leakage by application");

    Q_ASSERT(tables.count() == TableCount);

    MTSqlQuery inspections("SELECT circuits.refrigerant, circuits.field, inspections.refr_add_am, inspections.date"
                           " FROM inspections LEFT JOIN circuits ON inspections.circuit = circuits.id"
                           " AND inspections.customer = circuits.parent"
                           " WHERE (inspections.nominal <> 1 OR inspections.nominal IS NULL)");

    int current_year = QDate::currentDate().year();

    while (inspections.next()) {
        QString refrigerant = inspections.value(0).toString();
        QString field = inspections.value(1).toString();

        double refr_add_am = inspections.value(2).toDouble();
        if (refr_add_am == 0.0)
            continue;

        int year = inspections.value(3).toString().left(4).toInt();
        if (!weighted_averages && year > current_year)
            continue;

        if (min_year > year)
            min_year = year;
        if (max_year < year)
            max_year = year;

        addToValues(Key(year, refrigerant, field), RefrigerantAddition, refr_add_am);
        addToValues(Key(year, refrigerant, Key::All), RefrigerantAddition, refr_add_am);
        addToValues(Key(year, Key::All, field), RefrigerantAddition, refr_add_am);
        addToValues(Key(year), RefrigerantAddition, refr_add_am);
    }

    MTDictionary nominal_inspection_parents("nominal", "1");

    MTSqlQuery circuits("SELECT parent, id, refrigerant, field, refrigerant_amount, commissioning, decommissioning, disused FROM circuits");

    while (circuits.next()) {
        QString customer_id = circuits.stringValue("parent");
        QString circuit_id = circuits.stringValue("id");
        QString refrigerant = circuits.stringValue("refrigerant");
        QString field = circuits.stringValue("field");
        double refrigerant_amount = circuits.doubleValue("refrigerant_amount");

        int commissioning_year = circuits.stringValue("commissioning").left(4).toInt();
        if (commissioning_year > max_year)
            continue;

        int decommissioning_year = circuits.stringValue("decommissioning").left(4).toInt();
        if (circuits.intValue("disused") <= Circuit::Commissioned)
            decommissioning_year = 9999;
        else if (!decommissioning_year)
            decommissioning_year = current_year;

        if (decommissioning_year < min_year)
            continue;

        QVector<double> refrigerant_amounts(qMax(0, max_year - min_year) + 1);
        for (int year = qMax(commissioning_year, min_year); year <= max_year; ++year)
            refrigerant_amounts[year - min_year] = refrigerant_amount;

        nominal_inspection_parents.insert("customer", customer_id);
        nominal_inspection_parents.insert("circuit", circuit_id);
        ListOfVariantMaps nominal_inspections = MTRecord("inspections", "date", "", nominal_inspection_parents)
                .listAll("date, refr_add_am, refr_reco", "date ASC");

        foreach (const QVariantMap &nominal_inspection, nominal_inspections) {
            int nominal_inspection_year = nominal_inspection.value("date", "9999").toString().left(4).toInt();

            for (int year = qMax(nominal_inspection_year, min_year); year <= max_year; ++year)
                refrigerant_amounts[year - min_year] += nominal_inspection.value("refr_add_am", 0.0).toDouble()
                        - nominal_inspection.value("refr_reco", 0.0).toDouble();
        }

        for (int year = decommissioning_year + 1; year <= max_year; ++year)
            refrigerant_amounts[year - min_year] = 0.0;

        for (int year = min_year; year <= max_year; ++year) {
            double refrigerant_amount = refrigerant_amounts[year - min_year];
            addToValues(Key(year, refrigerant, field), RefrigerantAmount, refrigerant_amount);
            addToValues(Key(year, refrigerant, Key::All), RefrigerantAmount, refrigerant_amount);
            addToValues(Key(year, Key::All, field), RefrigerantAmount, refrigerant_amount);
            addToValues(Key(year), RefrigerantAmount, refrigerant_amount);
        }
    }

    QMap<Key, QVector<double> > weighted_average_components;

    QMutableMapIterator<Key, QVector<double> > i(values);
    while (i.hasNext()) { i.next();
        if (i.value()[RefrigerantAmount] != 0.0)
            i.value()[PercentageOfLeakage] = 100.0 * i.value()[RefrigerantAddition] / i.value()[RefrigerantAmount];

        QString refrigerant = i.key().refrigerant;
        QString refrigerant_name = attributeValues().value("refrigerant::" + refrigerant, QString());
        QString field = i.key().field;

        fields << field;

        if (weighted_averages) {
            Key key(refrigerant, field);

            if (!weighted_average_components[key].size())
                weighted_average_components[key].resize(2);

            weighted_average_components[key][0] += i.value()[PercentageOfLeakage] * i.value()[RefrigerantAmount];
            weighted_average_components[key][1] += i.value()[RefrigerantAmount];
        }

        if (!refrigerant.isEmpty() && refrigerant != field && refrigerant != Key::All)
            used_refrigerants.insert(refrigerant, refrigerant_name);
    }

    if (weighted_averages) {
        QMapIterator<Key, QVector<double> > i(weighted_average_components);
        while (i.hasNext()) { i.next();
            if (i.value().size() && i.value()[1]) {
                weighted_average_values.insert(i.key(), i.value()[0] / i.value()[1]);
            }
        }
    }
}

void LeakagesByApplication::addToValues(const Key &key, Table table, double value)
{
    if (!values[key].size())
        values[key].resize(TableCount);

    values[key][table] += value;
}
