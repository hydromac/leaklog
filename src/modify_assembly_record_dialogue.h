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

#ifndef MODIFY_ASSEMBLY_RECORD_DIALOGUE_H
#define MODIFY_ASSEMBLY_RECORD_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyAssemblyRecordDialogue : public TabbedModifyDialogue
{
    Q_OBJECT

public:
    ModifyAssemblyRecordDialogue(DBRecord *, QWidget * = NULL);

};

class ModifyAssemblyRecordDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyAssemblyRecordDialogueTab(int, QWidget * = NULL);

    void save(const QVariant &);
    QWidget * widget() { return this; }

private:
    void init();

    int record_id;
    QTreeWidget * tree;
};

#endif // MODIFY_ASSEMBLY_RECORD_DIALOGUE_H