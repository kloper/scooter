// -*- C++ -*-
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// This file is a part of the Scooter project. 
//
// Copyright (c) 2004. Dimitry Kloper <dimka@cs.technion.ac.il> . 
// Technion, Israel Institute of Technology. Computer Science Department.
//
// crApp.h -- wxApp instance for Chromium
//

#ifndef _crApp_h_
#define _crApp_h_

#include "crConfig.h"

#include <wx/app.h>

#include <dgDebug.h>

class wxDocManager;
class CrMainWindow;
class wxConfig;

class CrApp: public wxApp {
   public:
      CrApp(void);
      
   public:
      
      bool OnInit(void);
      int OnExit(void);
      void OnFatalException();

      wxDocManager *GetDocManager();
      CrMainWindow *GetMainWindow();
      wxConfig     *GetConfig();

   protected:
      bool ParseCmdLine();

   private:
      
      wxDocManager *m_doc_manager;
      CrMainWindow *m_main_window;
      wxConfig     *m_config;
      DGD::Debug::debug_factory_ref m_dout;
};

DECLARE_APP(CrApp)


#endif /* _crApp_h_ */

//
// crApp.h -- end of file
//

