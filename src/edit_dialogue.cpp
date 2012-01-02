/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "edit_dialogue.h"
#include "input_widgets.h"
#include "records.h"
#include "edit_dialogue_layout.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

void EditDialogue::init(DBRecord * record)
{
    md_record = record;
    QVBoxLayout * md_vlayout_main = new QVBoxLayout(this);
    md_vlayout_main->setSpacing(9);
    md_vlayout_main->setContentsMargins(9, 9, 9, 9);
    md_grid_main = new QGridLayout;
    addMainGridLayout(md_vlayout_main);
    QDialogButtonBox * md_bb = new QDialogButtonBox(this);
    md_bb->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    md_bb->button(QDialogButtonBox::Save)->setText(tr("Save"));
    md_bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    QObject::connect(md_bb, SIGNAL(accepted()), this, SLOT(save()));
    QObject::connect(md_bb, SIGNAL(rejected()), this, SLOT(reject()));
    md_vlayout_main->addWidget(md_bb);
    this->resize(20, 20);
}

void EditDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    md_vlayout_main->addLayout(md_grid_main);
}

EditDialogue::EditDialogue(QWidget * parent):
QDialog(parent)
{}

EditDialogue::EditDialogue(DBRecord * record, QWidget * parent):
QDialog(parent)
{
    init(record);

    md_record->initEditDialogue(this);

    EditDialogueColumnLayout(&md_inputwidgets, md_grid_main).layout();
}

void EditDialogue::setWindowTitle(const QString & title)
{
    if (!md_record->id().isEmpty()) {
        this->QDialog::setWindowTitle(tr("%1: %2").arg(title).arg(md_record->id()));
    } else {
        this->QDialog::setWindowTitle(title);
    }
}

void EditDialogue::save()
{
    save(true);
}

bool EditDialogue::save(bool call_accept)
{
    QVariantMap values; QString id; QVariant value;
    for (QList<MDAbstractInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        if ((*i)->skipSave()) continue;
        id = (*i)->id();
        value = (*i)->variantValue();
        if (id == md_record->idField()) {
            if (value.toString().isEmpty()) {
                QMessageBox::information(NULL, tr("Save changes"), tr("Invalid ID."));
                return false;
            } else if (md_used_ids.contains(value.toString())) {
                if (id == "date") {
                    QMessageBox::information(NULL, tr("Save changes"), tr("This date is not available. Please choose a different date."));
                } else {
                    QMessageBox::information(NULL, tr("Save changes"), tr("This ID is not available. Please choose a different ID."));
                }
                return false;
            }
        }
        values.insert(id, value);
    }
    if (!md_record->checkValues(values, this))
        return false;
    md_record->update(values, true);

    if (call_accept) accept();
    return true;
}

const QVariant EditDialogue::idFieldValue()
{
    for (int i = 0; i < md_inputwidgets.count(); ++i) {
        if (md_inputwidgets.at(i)->id() == "id") {
            return md_inputwidgets.at(i)->variantValue();
        }
    }
    return QVariant();
}

MDAbstractInputWidget * EditDialogue::inputWidget(const QString id)
{
    for (int i = 0; i < md_inputwidgets.count(); ++i) {
        if (md_inputwidgets.at(i)->id() == id) {
            return md_inputwidgets.at(i);
        }
    }
    return NULL;
}
