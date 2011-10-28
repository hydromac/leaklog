/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "variable_evaluation.h"

#include "variables.h"
#include "global.h"
#include "records.h"

using namespace Global;

VariableEvaluation::EvaluationContext::EvaluationContext(const QString & customer_id, const QString & circuit_id, int vars_scope):
    customer_id(customer_id),
    circuit_id(circuit_id),
    vars_scope(vars_scope)
{
    init();
}

VariableEvaluation::EvaluationContext::~EvaluationContext()
{
    QMapIterator<QString, VariableEvaluation::Variable *> i(vars_map);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
}

void VariableEvaluation::EvaluationContext::init()
{
    Variables vars(QSqlDatabase(), vars_scope);
    VariableEvaluation::Variable * parent_var, * var;

    while (vars.next()) {
        var = vars_map.value(vars.id(), NULL);
        if (!var) {
            var = new VariableEvaluation::Variable;
            vars_map.insert(vars.id(), var);
            vars_list.append(var);
        }

        var->setId(vars.id());
        var->setName(vars.name());
        var->setType(vars.type());
        var->setUnit(vars.unit());
        var->setValue(vars.valueExpression());
        var->setCompareNom(vars.compareNom());
        var->setColBg(vars.colBg());
        var->setTolerance(vars.tolerance());

        if (!vars.parentID().isEmpty()) {
            parent_var = vars_map.value(vars.parentID(), NULL);
            if (!parent_var) {
                parent_var = new VariableEvaluation::Variable;
                vars_map.insert(vars.parentID(), parent_var);
            }

            parent_var->addSubvariable(var);
        }
    }

    used_ids = listVariableIds();
}

QString VariableEvaluation::EvaluationContext::variableName(Variable * var, bool is_nominal)
{
    if (is_nominal && var->id() == "refr_add_am")
        return QObject::tr("New charge");

    return var->name();
}

QString VariableEvaluation::EvaluationContext::evaluate(const QString & var_name, QVariantMap & inspection, QString & nom_value)
{
    VariableEvaluation::Variable * var = vars_map.value(var_name);
    if (!var) return QString();
    return var->evaluate(customer_id, circuit_id, inspection, used_ids, nominal_ins, nom_value);
}

QString VariableEvaluation::EvaluationContext::evaluate(VariableEvaluation::Variable * var, QVariantMap & inspection, QString & nom_value)
{
    return var->evaluate(customer_id, circuit_id, inspection, used_ids, nominal_ins, nom_value);
}

QString VariableEvaluation::Variable::evaluate(const QString & customer_id, const QString & circuit_id, QVariantMap & inspection, QStringList & used_ids, QVariantMap & nominal_ins, QString & nom_value)
{
    QString ins_value = inspection.value(id()).toString();

    if (value().isEmpty()) {
        if (!ins_value.isEmpty()) {
            if (id() == "inspector") {
                Inspector inspector(ins_value);
                ins_value = inspector.stringValue("person", ins_value);
            } else if (id() == "operator") {
                if (ins_value.toInt())
                    ins_value = Person(ins_value, customer_id).stringValue("name", ins_value);
            }
        }

        if (compareNom()) {
            nom_value = nominal_ins.value(id()).toString();
        }
    } else {
        MTDictionary expression = parseExpression(value(), used_ids);
        bool ok_eval, is_null;
        ins_value = QString::number(evaluateExpression(inspection, expression, customer_id, circuit_id, &ok_eval, &is_null));
        if (!ok_eval || is_null) ins_value.clear();

        if (nominal_ins.isEmpty()) nom_value.clear();
        else if (compareNom()) {
            nom_value = QString::number(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
            if (!ok_eval) nom_value.clear();
        }
    }

    return ins_value;
}
