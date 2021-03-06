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

#include "expression.h"

#include "global.h"
#include "refprop.h"
#include "mtsqlquery.h"
#include "mtrecord.h"

#include <QRegExp>
#include <QStringList>

#include <cmath>

using namespace Global;

class PressureToTemperatureFunction : public FunctionParser::FunctionWrapper
{
public:
    PressureToTemperatureFunction(RefProp::Phase phase): phase(phase) {}

    void setRefrigerant(const QString &refrigerant) {
        this->refrigerant = refrigerant;
    }

    double callFunction(const double *values) {
        return RefProp::pressureToTemperature(refrigerant, values[0], phase);
    }

private:
    QString refrigerant;
    RefProp::Phase phase;
};

Expression::Expression(const QString &expression):
fparser(new FunctionParser)
{
    fparser->AddFunctionWrapper("p_to_t", PressureToTemperatureFunction(RefProp::Liquid), 1);
    fparser->AddFunctionWrapper("p_to_t_vap", PressureToTemperatureFunction(RefProp::Vapour), 1);

    QString exp = expression;
    exp.replace(QRegExp("\\bsum\\s*\\(\\s*"), "(__sum_");

    std::string vars;
    if (fparser->ParseAndDeduceVariables(exp.toStdString(), vars) < 0) {
        var_names = QSharedPointer<QStringList>(new QStringList(QString::fromStdString(vars).split(',', QString::SkipEmptyParts)));
    }
}

Expression::~Expression() {}

double Expression::evaluate(const QVariantMap &inspection, const QString &customer_id, const QString &circuit_id, bool *ok, bool *null_var) const
{
    MTRecord circuit("circuits", "id", circuit_id, MTDictionary("parent", customer_id));
    QVariantMap circuit_attributes = circuit.list("*, " + circuitRefrigerantAmountQuery());
    return evaluate(inspection, circuit_attributes, ok, null_var);
}

double Expression::evaluate(const QVariantMap &inspection, const QVariantMap &circuit_attributes, bool *ok, bool *null_var) const
{
    static const QString sum_query("SELECT SUM(CAST(%1 AS numeric)) FROM inspections"
                                   " WHERE date LIKE '%2%' AND customer = :customer_id AND circuit = :circuit_id"
                                   " AND (nominal <> 1 OR nominal IS NULL)");

    if (null_var) *null_var = false;

    if (!var_names) {
        if (ok) *ok = false;
        return 0;
    }

    if (ok) *ok = true;

    QString inspection_date = inspection.value("date").toString();
    QString refrigerant = circuit_attributes.value("refrigerant").toString();

    static_cast<PressureToTemperatureFunction *>(fparser->GetFunctionWrapper("p_to_t"))->setRefrigerant(refrigerant);
    static_cast<PressureToTemperatureFunction *>(fparser->GetFunctionWrapper("p_to_t_vap"))->setRefrigerant(refrigerant);

    double *values = var_names->count() ? new double[var_names->count()] : NULL;
    for (int i = 0; i < var_names->count(); ++i) {
        values[i] = 0.0;
        QString var_name = var_names->at(i);
        if (var_name.startsWith("__sum_")) {
            var_name.remove(0, 6);
            MTSqlQuery sum_ins;
            sum_ins.prepare(sum_query.arg(var_name).arg(inspection_date.left(4)));
            sum_ins.bindValue(":customer_id", circuit_attributes.value("parent"));
            sum_ins.bindValue(":circuit_id", circuit_attributes.value("id"));
            if (sum_ins.exec() && sum_ins.next())
                values[i] = sum_ins.value(0).toDouble();
        } else if (inspection.contains(var_name)) {
            QVariant value = inspection.value(var_name);
            values[i] = value.toDouble();
            if (null_var && value.isNull()) *null_var = true;
        } else if (circuit_attributes.contains(var_name)) {
            QVariant value = circuit_attributes.value(var_name);
            values[i] = value.toDouble();
            if (null_var && value.isNull()) *null_var = true;
        } else if (var_name == "gwp") {
            values[i] = Global::refrigerantGWP(refrigerant);
        } else if (var_name == "co2_equivalent") {
            values[i] = Global::CO2Equivalent(refrigerant, circuit_attributes.value("refrigerant_amount").toDouble());
        } else {
            if (ok) *ok = false;
            if (null_var) *null_var = true;
        }
    }

    long double result = fparser->Eval(values);
    delete[] values;

    if (round(result) == result)
        return (double)result;
    return (double)(round(result * REAL_NUMBER_PRECISION_EXP) / REAL_NUMBER_PRECISION_EXP);
}
