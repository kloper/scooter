// -*- C++ -*-
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
// Copyright (c) Dimitry Kloper <kloper@users.sf.net> 2002-2012
// 
// This file is a part of the Scooter project. 
// 
// boxfish_portal.cpp -- boxfish main window
// 

#include <iostream>
#include <string>

#include <boost/shared_array.hpp>

#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtCore/QSignalMapper>
#include <QtCore/QDir>
#include <QtCore/QUrl>

#include <QtGui/QMainWindow>
#include <QtGui/QWorkspace>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>

#include <dgd.h>

#include <openvrml/browser.h>

#include "boxfish_trace.h"
#include "boxfish_cfg.h"
#include "boxfish_portal.h"
#include "boxfish_svg.h"
#include "boxfish_vrml_control.h"
#include "boxfish_document.h"
#include "boxfish_console.h"

namespace boxfish {

Portal::Portal() :
   QMainWindow(),
   m_workspace(NULL),
   m_open_mapper(NULL),
   m_history_mapper(NULL),
   m_activation_mapper(NULL),
   m_properties_mapper(NULL),
   m_filehist_menu(NULL),
   m_file_menu(NULL),
   m_window_menu(NULL),
   m_help_menu(NULL),
   m_file_toolbar(NULL),
   m_render_toolbar(NULL),
   m_open_action(NULL),
   m_help_action(NULL),
   m_exit_action(NULL),
   m_tile_action(NULL),
   m_cascade_action(NULL),
   m_close_action(NULL),
   m_flat_action(NULL),
   m_phong_action(NULL),
   m_wireframe_action(NULL),
   m_center_action(NULL),
   m_culling_action(NULL),
   m_texture_action(NULL),
   m_shading_actions(NULL),
   m_open_dialog(NULL),
   m_tool_docker(NULL),
   m_tool_tab(NULL),
   m_console(NULL),
   m_cout_buffer(NULL),
   m_cerr_buffer(NULL)
{
   m_workspace = new QWorkspace;
   m_workspace->setScrollBarsEnabled(true);

   connect( m_workspace, SIGNAL(windowActivated(QWidget*)),
	    this, SLOT(window_activated(QWidget*)) );

   connect( m_workspace, SIGNAL(windowActivated(QWidget*)),
	    this, SLOT(handle_glpad_propchange(QWidget*)) );

   this->setCentralWidget(m_workspace);

   m_open_mapper = new QSignalMapper(this);
   connect( m_open_mapper, SIGNAL(mapped(const QString &)),
	    this, SLOT(open(const QString &)));

   m_history_mapper = new QSignalMapper(this);
   connect( m_history_mapper, SIGNAL(mapped(const QString &)),
	    this, SLOT(update_history(const QString &)));

   m_activation_mapper = new QSignalMapper(this);
   connect( m_activation_mapper, SIGNAL(mapped(QWidget*)),
	    m_workspace, SLOT(setActiveWindow(QWidget*)) );

   m_properties_mapper = new QSignalMapper(this);
   connect( m_properties_mapper, SIGNAL(mapped(QWidget*)),
	    this, SLOT(handle_glpad_propchange(QWidget*)) );

   int isize = Config::main()->get( "icon::size" ).toInt();
   this->setIconSize( QSize( isize, isize ) );
   
   construct_actions();
   construct_menus();
   construct_toolbars();
   construct_dialogs();
   construct_dockers();
   construct_console();

   set_geometry();
   set_window_state();

   this->setWindowTitle( tr("Boxfish3d") );
   this->setWindowIcon( Svg_icon( ":/icons/boxfish3d.svg", QSize(32,32 ) ) );

   if( Config::main()->defined( "portal::state" ) ) {
      this->restoreState( Config::main()->get( "portal::state" ).toUtf8() );
   }

   int count = 0;
   QString fname;
   do {
      fname = Config::main()->get( QString("file::history::list::item") +
				   QString::number(count) );
      count++;

      if( !fname.isNull() ) 
	 m_file_history.push_back( fname );
            
   } while( !fname.isNull() );
}

Portal::~Portal() {
   if( m_cout_buffer != NULL ) 
      std::cout.rdbuf(m_cout_buffer);
   if( m_cerr_buffer != NULL )
      std::cerr.rdbuf(m_cerr_buffer);

   Config::main()->set( "portal::state", QString(this->saveState()) );

   Qt::WindowStates window_state = this->windowState();
   int maximized =  ( (window_state & Qt::WindowMaximized) != 0 ) ? 1 : 0;

   Config::main()->set( "portal::geometry::maximized", 
                        QString::number(maximized) );

   QRect rect = this->normalGeometry();
   Config::main()->set( "portal::geometry::left", 
			QString::number( rect.left() ) );
   Config::main()->set( "portal::geometry::right", 
			QString::number( rect.right() ) );
   Config::main()->set( "portal::geometry::top", 
			QString::number( rect.top() ) );
   Config::main()->set( "portal::geometry::bottom", 
			QString::number( rect.bottom() ) );

   Config::main()->set( QString("portal::dialogs::open::cwd"), 
			m_open_dialog->directory().absolutePath() );

   int count = 0;
   for( QStringList::const_iterator fiter = m_file_history.begin();
	fiter != m_file_history.end();
	++fiter ) {
      Config::main()->set( QString("file::history::list::item") +
			   QString::number(count),
			   *fiter );
      count++;
   }

   if( m_workspace ) {
      delete m_workspace;
      m_workspace = NULL;
   }
}

void Portal::closeEvent(QCloseEvent *event) {
   m_workspace->closeAllWindows();
}

void Portal::open( const QString& fname ) {
   dgd_scopef(trace_gui);

   dgd_echo(fname);

   QUrl url = QUrl::fromUserInput( fname );
   
   dgd_echo( QString(url.toEncoded()) );

   if( url.isValid() ) {
      Document *doc = new Document( url );

      QRect wsgeom = m_workspace->geometry();
      QRect mygeom;

      mygeom.setLeft( 10 );
      mygeom.setTop( 35 );
      mygeom.setWidth( (int)(wsgeom.width() * 0.7) );
      mygeom.setHeight( (int)(wsgeom.height() * 0.7) );

      doc->setGeometry( mygeom );

      connect( doc, SIGNAL(load_failed()), doc, SLOT(close()) );
      connect( doc, SIGNAL(load_finished()), 
	       m_history_mapper, SLOT(map()) );
      connect( doc, SIGNAL(load_finished()), 
	       m_properties_mapper, SLOT(map()) );

      m_history_mapper->setMapping( doc, QString(url.toEncoded()) );
      m_properties_mapper->setMapping( doc, doc );

      m_workspace->addWindow( doc );

      doc->show();
   }
}

void Portal::open() {
   dgd_scopef(trace_gui);

   QString fname;

   if( m_open_dialog->exec() !=  QDialog::Accepted ) {
      return;
   }
      
   QStringList selected_files = m_open_dialog->selectedFiles();
   
   for( QStringList::const_iterator fiter = selected_files.begin();
	fiter != selected_files.end();
	++fiter ) {
      dgd_echo(*fiter);
      this->open( *fiter );
   }
}

void Portal::close() {
   QWidget *current = m_workspace->activeWindow();
   if( current != NULL )
      current->close();
}

void Portal::help() {
}

void Portal::construct_dialogs() {
   QString cwd = Config::main()->get( "portal::dialogs::open::cwd" );
   if( cwd.isNull() )
      cwd = QDir::homePath();

   m_open_dialog = new QFileDialog( this, 
				    tr("Open File"),
				    cwd,
				    tr("VRML Files (*.wrl *.vrml);;"
                                       "X3D Files (*.x3d *.x3dv)") );

   m_open_dialog->setViewMode( QFileDialog::Detail );
}

void Portal::construct_actions() {
   m_open_action = new QAction( Svg_icon( ":/icons/open.svg", 
					  this->iconSize() ), 
				tr("&Open"), 
				this);
   m_open_action->setShortcut(tr("Ctrl+F"));
   m_open_action->setStatusTip(tr("Open an existing file"));
   connect(m_open_action, SIGNAL(triggered()), this, SLOT(open()));

   m_close_action = new QAction( Svg_icon( ":/icons/close.svg", 
                                           this->iconSize() ), 
				 tr("&Close"), 
				 this);
   m_close_action->setShortcut(tr("Ctrl+F4"));
   m_close_action->setStatusTip(tr("Close window"));
   connect(m_close_action, SIGNAL(triggered()), this, SLOT(close()));
   
   m_help_action = new QAction( Svg_icon( ":/icons/help_book.svg", 
					  this->iconSize() ), 
				tr("&Help"), 
				this);
   m_help_action->setShortcut(tr("F1"));
   m_help_action->setStatusTip(tr("Help me out there!"));
   connect(m_help_action, SIGNAL(triggered()), this, SLOT(help()));

   m_exit_action = new QAction( Svg_icon( ":/icons/exit.svg", 
					  this->iconSize() ), 
				tr("E&xit"), 
				this);
   m_exit_action->setStatusTip(tr("Exit application"));
   connect(m_exit_action, SIGNAL(triggered()), 
	   (QApplication*)QApplication::instance(), SLOT(closeAllWindows()));

   m_tile_action = new QAction( Svg_icon( ":/icons/tile_horiz.svg", 
                                          this->iconSize() ), 
                                tr("&Tile"), 
                                this);
   m_tile_action->setStatusTip(tr("Tile windows"));
   connect(m_tile_action, SIGNAL(triggered()), m_workspace, SLOT(tile()));

   m_cascade_action = new QAction( Svg_icon( ":/icons/cascade.svg", 
					     this->iconSize() ), 
				   tr("&Cascade"), 
				   this);
   m_cascade_action->setStatusTip(tr("Cascade windows"));
   connect(m_cascade_action, SIGNAL(triggered()), 
	   m_workspace, SLOT(cascade()));

   m_flat_action = new QAction( Svg_icon( ":/icons/flat.svg",
					  this->iconSize() ),
				tr("&Flat"),
				this );
   m_flat_action->setCheckable( true );
   m_flat_action->setChecked( false );
   m_flat_action->setStatusTip(tr("Flat shading"));
   connect( m_flat_action, SIGNAL(toggled(bool)),
	    this, SLOT(handle_glpad_command()) );

   m_phong_action = new QAction( Svg_icon( ":/icons/phong.svg",
                                           this->iconSize() ),
                                 tr("&Phong"),
                                 this );
   m_phong_action->setCheckable( true );
   m_phong_action->setChecked( true );
   m_phong_action->setStatusTip(tr("Phong shading"));
   connect( m_phong_action, SIGNAL(toggled(bool)),
	    this, SLOT(handle_glpad_command()) );
   
   m_wireframe_action = new QAction( Svg_icon( ":/icons/wireframe.svg",
					       this->iconSize() ),
				     tr("&Wireframe"),
				     this );
   m_wireframe_action->setCheckable( true );
   m_wireframe_action->setChecked( false );
   m_wireframe_action->setStatusTip(tr("Show wireframe"));
   connect( m_wireframe_action, SIGNAL(toggled(bool)),
	    this, SLOT(handle_glpad_command()) );

   m_center_action = new QAction( Svg_icon( ":/icons/center.svg",
					    this->iconSize() ),
				  tr("&Center"),
				  this );
   m_center_action->setObjectName(tr("center"));
   m_center_action->setStatusTip(tr("Center scene"));
   connect( m_center_action, SIGNAL(triggered()),
	    this, SLOT(handle_glpad_command()) );

   m_shading_actions = new QActionGroup(this);
   m_shading_actions->addAction( m_wireframe_action );
   m_shading_actions->addAction( m_flat_action );
   m_shading_actions->addAction( m_phong_action );

   m_culling_action = new QAction( Svg_icon( ":/icons/culling.svg",
					     this->iconSize() ),
				   tr("&Back Face Culling"),
				   this );
   m_culling_action->setCheckable( true );
   m_culling_action->setChecked( true );
   m_culling_action->setStatusTip(tr("Toggle back-face culling"));
   connect( m_culling_action, SIGNAL(toggled(bool)),
	    this, SLOT(handle_glpad_command()) );

   m_texture_action = new QAction( Svg_icon( ":/icons/texture.svg",
					     this->iconSize() ),
				   tr("&Texture Rendering"),
				   this );
   m_texture_action->setCheckable( true );
   m_texture_action->setChecked( true );
   m_texture_action->setStatusTip(tr("Toggle texture rendering"));
   connect( m_texture_action, SIGNAL(toggled(bool)),
	    this, SLOT(handle_glpad_command()) );

}

void Portal::construct_menus() {
   m_filehist_menu = new QMenu(tr("&Recent Files"), this);
   connect( m_filehist_menu, SIGNAL(aboutToShow()), 
	    this, SLOT(construct_filehist_menu()) );

   m_file_menu = this->menuBar()->addMenu(tr("&File"));
   m_file_menu->addAction( m_open_action );
   m_file_menu->addSeparator();
   m_file_menu->addAction( m_close_action );
   m_file_menu->addSeparator();
   m_file_menu->addMenu( m_filehist_menu );
   m_file_menu->addSeparator();
   m_file_menu->addAction( m_exit_action );
   
   m_window_menu = this->menuBar()->addMenu(tr("&Window"));
   connect( m_window_menu, SIGNAL(aboutToShow()), 
	    this, SLOT(construct_window_menu()) );
   
   m_help_menu = this->menuBar()->addMenu(tr("&Help"));
   m_help_menu->addAction( m_help_action );
}

void Portal::construct_toolbars() {
   m_file_toolbar = this->addToolBar(tr("File"));
   m_file_toolbar->setObjectName("fileToolBar");
   m_file_toolbar->addAction( m_open_action );
   m_file_toolbar->addSeparator();
   m_file_toolbar->addAction( m_close_action );

   m_render_toolbar = this->addToolBar(tr("Render"));
   m_render_toolbar->setObjectName("renderToolBar");
   m_render_toolbar->addAction( m_center_action );
   m_render_toolbar->addSeparator();
   m_render_toolbar->addActions( m_shading_actions->actions() );
   m_render_toolbar->addSeparator();
   m_render_toolbar->addAction( m_culling_action );
   m_render_toolbar->addAction( m_texture_action );
}

void Portal::construct_filehist_menu() {   
   int count = 0;

   m_filehist_menu->clear();
   for( QStringList::const_iterator hiter = m_file_history.begin();
	hiter != m_file_history.end(); 
	++hiter ) {
      QAction *action = 
	 m_filehist_menu->addAction( *hiter, m_open_mapper, SLOT(map()), 
				     QKeySequence(tr("Ctrl+") +
						  QString::number(count)) );
      m_open_mapper->setMapping( action, *hiter );
      ++count;
   }
}

void Portal::construct_window_menu() {   
   m_window_menu->clear();
   m_window_menu->addAction( m_close_action );
   m_window_menu->addSeparator();
   m_window_menu->addAction( m_tile_action );
   m_window_menu->addAction( m_cascade_action );


   QWidgetList windows = m_workspace->windowList();

   if( windows.size() > 0 ) 
      m_window_menu->addSeparator();

   for( QWidgetList::const_iterator witer = windows.begin();
	witer != windows.end(); 
	++witer ) {
      QWidget *widget = *witer;
      QAction *action = 
	 m_window_menu->addAction( widget->windowTitle(), 
				   m_activation_mapper, SLOT(map()) );
      m_activation_mapper->setMapping( action, widget );
   }
}

void Portal::construct_dockers() {
   m_tool_docker = new QDockWidget( tr("Tool Pane"), this );
   m_tool_docker->setObjectName("toolPaneDocker");

   this->addDockWidget(Qt::BottomDockWidgetArea, m_tool_docker);

   m_tool_tab = new QTabWidget(m_tool_docker);
   m_tool_tab->setTabShape( QTabWidget::Rounded );
   m_tool_tab->setTabPosition( QTabWidget::East );
   
   m_tool_docker->setWidget( m_tool_tab );
}

void Portal::construct_console() {
   m_console = new Console(m_tool_tab);

   m_tool_tab->addTab(m_console, tr("Console"));

   typedef boost::iostreams::stream_buffer<Console_sink> console_buffer;

   console_buffer *cout_buffer = 
      new console_buffer(m_console, std::string("cout"));
   console_buffer *cerr_buffer =
      new console_buffer(m_console, std::string("cerr"));

   m_cout_buffer = std::cout.rdbuf();
   m_cerr_buffer = std::cerr.rdbuf();

   std::cout.rdbuf(cout_buffer);
   std::cerr.rdbuf(cerr_buffer);
}

QRect Portal::default_geometry() const {
   QRect desktop_size = QApplication::desktop()->screenGeometry( this );
   desktop_size.adjust( 10, 30, -10, -30 );

   return desktop_size;
}

void Portal::set_window_state(Qt::WindowStates state) 
{
   QString val = Config::main()->get("portal::geometry::maximized");
   if( !val.isNull() ) {
      if( val.toInt() != 0 ) {
         state |= Qt::WindowMaximized;
      }
   }
   this->setWindowState( state );
}

void Portal::set_geometry( QRect rect ) {
   if( rect.isNull() ) {
      rect = this->default_geometry();
      
      QString val;

      val = Config::main()->get( "portal::geometry::left" );
      if( !val.isNull() ) 
	 rect.setLeft( val.toInt() );
      val = Config::main()->get( "portal::geometry::right" );
      if( !val.isNull() ) 
	 rect.setRight( val.toInt() );
      val = Config::main()->get( "portal::geometry::top" );
      if( !val.isNull() ) 
	 rect.setTop( val.toInt() );
      val = Config::main()->get( "portal::geometry::bottom" );
      if( !val.isNull() ) 
	 rect.setBottom( val.toInt() );
   }

   if( !rect.isValid() )
      rect = this->default_geometry();

   this->setGeometry( rect );
}

void Portal::update_history( const QString &fname ) {
   dgd_scopef(trace_gui);

   dgd_echo(fname);
   
   for( QStringList::iterator hiter = m_file_history.begin();
	hiter != m_file_history.end(); ) {
      if( *hiter == fname ) 
	 hiter = m_file_history.erase( hiter );
      else
	 ++hiter;
   }
   
   dgd_echo(Config::main()->get( "file::history::size" ).toInt());
   
   if( m_file_history.size() >= 
       Config::main()->get( "file::history::size" ).toInt() ) 
      m_file_history.pop_front();
   
   m_file_history.push_back( fname );
   
   Config::main()->set( QString("portal::dialogs::open::cwd"), fname );
}

void Portal::window_activated( QWidget *w ) {
   dgd_scopef(trace_gui);

   Document *doc = dynamic_cast<Document*>(w);

   if( doc != NULL ) {
      dgd_echo(doc->windowTitle());

      QString scene_tree_tab_name( tr("Scene Tree") );
      for(int i = 0; i < m_tool_tab->count(); i++ ) {
         if( m_tool_tab->tabText(i) == scene_tree_tab_name ) {
            m_tool_tab->removeTab(i);
            break;
         }
      }

      QWidget *tool_widget = doc->toolset();

      if( tool_widget != NULL ) {
         m_tool_tab->setCurrentIndex(
            m_tool_tab->addTab(tool_widget, scene_tree_tab_name)
         );
      } 
   }
}

void Portal::handle_glpad_propchange( QWidget *w ) {
   dgd_scopef(trace_gui);

   Document *doc = dynamic_cast<Document*>(w);

   if( doc != NULL ) {
      dgd_echo(doc->windowTitle());

      m_flat_action->setChecked( 
	 doc->glpad_property("shading_mode").toInt() == vrml::Control::FLAT &&
	 doc->glpad_property("polygon_mode").toInt() != 
         vrml::Control::WIREFRAME 
      );

      m_phong_action->setChecked( 
	 doc->glpad_property("shading_mode").toInt() == vrml::Control::SMOOTH 
	 &&
	 doc->glpad_property("polygon_mode").toInt() != 
         vrml::Control::WIREFRAME
      );

      m_wireframe_action->setChecked( 
	 doc->glpad_property("polygon_mode").toInt() == 
         vrml::Control::WIREFRAME 
      );

      m_culling_action->setChecked( 
	 doc->glpad_property("back_face_culling").toBool()
      );

      m_texture_action->setChecked( 
	 doc->glpad_property("texture_mapping").toBool()
      );

   }
}

void Portal::handle_glpad_command() {
   QObject *sender = this->sender();

   if( sender == m_flat_action && m_flat_action->isChecked() ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_property( "shading_mode", vrml::Control::FLAT );
	 doc->glpad_property( "polygon_mode", vrml::Control::FILLED );
      }
   }

   if( sender == m_phong_action && m_phong_action->isChecked() ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_property( "shading_mode", vrml::Control::SMOOTH );
	 doc->glpad_property( "polygon_mode", vrml::Control::FILLED );
      }
   }

   if( sender == m_wireframe_action && m_wireframe_action->isChecked() ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_property( "shading_mode", vrml::Control::SMOOTH );
	 doc->glpad_property( "polygon_mode", vrml::Control::WIREFRAME );
      }
   }

   if( sender == m_culling_action ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_property( "back_face_culling", 
			      m_culling_action->isChecked() );
      }
   }

   if( sender == m_texture_action ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_property( "texture_mapping", 
			      m_texture_action->isChecked() );
      }
   }

   if( sender == m_center_action ) {
      Document *doc = dynamic_cast<Document*>( m_workspace->activeWindow() );
      if( doc != NULL ) {
	 doc->glpad_reset();
      }
   }
}

}; // end of namespace boxfish


// 
// boxfish_portal.cpp -- end of file
//


