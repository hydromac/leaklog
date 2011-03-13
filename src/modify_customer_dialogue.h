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

#ifndef MODIFY_CUSTOMER_DIALOGUE_H
#define MODIFY_CUSTOMER_DIALOGUE_H

#include "modify_dialogue.h"

class Customer;
class ModifyDialogueBasicTable;

class ModifyCustomerDialogue : public ModifyDialogue
{
    Q_OBJECT

public:
    ModifyCustomerDialogue(Customer *, QWidget * = NULL);

protected slots:
    void save();

private:
    ModifyDialogueBasicTable * persons_table;
};

#endif // MODIFY_CUSTOMER_DIALOGUE_H