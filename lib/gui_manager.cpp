//  Flexlay - A Generic 2D Game Editor
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2024 The NetPanzer Team (https://github.com/netpanzer/)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <ClanLib/gui.h>
#include <ClanLib/guistylesilver.h>
#include <ClanLib/core.h>

#include "globals.hpp"
#include "gui_manager.hpp"

GUIManager* GUIManager::current_ = 0;

class GUIManagerImpl
{
public:
  std::stack<CL_Component*> components;

  CL_GUIManager*      manager;
  CL_StyleManager*    style;
  CL_ResourceManager* resources;
  CL_SlotContainer*   slot_container;
};

GUIManager::GUIManager()
  : impl(new GUIManagerImpl())
{
  std::cout << "Creating GUIManager: " << datadir + "/gui/gui.xml" << std::endl;
  impl->slot_container = new CL_SlotContainer();
  impl->resources = new CL_ResourceManager(datadir + "/gui/gui.xml");
  impl->style     = new CL_StyleManager_Silver(impl->resources);
  impl->manager   = new CL_GUIManager(impl->style);
  current_  = this;

  // Make the manager the first component on the stack
  push_component(impl->manager);
}

GUIManager::~GUIManager()
{
  pop_component();

  delete impl->manager;
  //delete style; FIXME: Memory hole?!
  //delete resources;  FIXME: Memory hole?!
  delete impl->slot_container;
}

void
GUIManager::draw()
{
  if (impl->manager->is_input_enabled())
    impl->manager->show();
}

void
GUIManager::update()
{
  // nothing to do
}

void
GUIManager::run()
{
  impl->manager->run();
}

CL_Component*
GUIManager::get_component()
{
  return impl->components.top();
}

CL_SlotContainer*
GUIManager::get_slot_container()
{
  return impl->slot_container;
}

void
GUIManager::hide()
{
  if (impl->manager->is_input_enabled())
    impl->manager->disable_input();
}

void
GUIManager::show()
{
  if (!impl->manager->is_input_enabled())
    impl->manager->enable_input();
}

bool
GUIManager::is_visible()
{
  return impl->manager->is_input_enabled();
}

void
GUIManager::quit()
{
  impl->manager->quit();
}

void
GUIManager::push_component(CL_Component* c)
{
  impl->components.push(c);
}

void
GUIManager::pop_component()
{
  impl->components.pop();
}

/* EOF */
