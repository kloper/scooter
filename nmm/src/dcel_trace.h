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
// Copyright (c) 2005. Dimitry Kloper <dimka@cs.technion.ac.il> . 
// Technion, Israel Institute of Technology. Computer Science Department.
//
// dcel_trace.h -- output operators for nmm::Dcel
//

#ifndef _dcel_trace_h_
#define _dcel_trace_h_

#include <iostream>

#include <boost/iterator/transform_iterator.hpp>

#include <dgDebug.h>


namespace scooter {
namespace nmm {
namespace trace {

using namespace DGD;

template <class T> 
class brief_type {
   public:
      typedef T value_type;

      brief_type( const T& b ) : m_base( b ) {}
      brief_type( const brief_type& peer ) : m_base( peer.m_base ) {}
      const T& m_base;
};

template <class T> 
class verbose_type {
   public:
      typedef T value_type;

      verbose_type( const T& b ) : m_base( b ) {};
      verbose_type( const verbose_type& peer ) : m_base( peer.m_base ) {}
      const T& m_base;
};

template <class T>
brief_type<T> brief( const T& base ) { return brief_type<T>(base); }

template <class T>
verbose_type<T> verbose( const T& base ) { return verbose_type<T>(base); }

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const brief_type<Dcel<Vb,Hb,Fb>::Vertex> &bv) {
   ostr << (const Dcel<Vb,Hb,Fb>::Vertex::Parent&)bv.m_base;
   return ostr;
}


template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const brief_type<Dcel<Vb,Hb,Fb>::Halfedge>& bhe ) {
   const Dcel<Vb,Hb,Fb>::Halfedge& he = bhe.m_base;
   if( he.opposite() == NULL || he.opposite()->vertex() == NULL ) 
      ostr << "null";
   else
      ostr << brief( *he.opposite()->vertex() );
   ostr << "->";
   if( he.vertex() == NULL ) 
      ostr << "null";
   else
      ostr << brief( *he.vertex() );
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const brief_type<Dcel<Vb,Hb,Fb>::Facet> &bf ) { 
   const Dcel<Vb,Hb,Fb>::Facet& f = bf.m_base;
   if( f.halfedge() != NULL ) 
      ostr << "<" << dgd << brief(*f.halfedge()) << ">";
   else
      ostr << "<null_facet>";
   return ostr;
}


template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const brief_type<Dcel<Vb,Hb,Fb> > &bdcel ) {
   const Dcel<Vb,Hb,Fb>& dcel = bdcel.m_base;
   ostr << "{" << dgd << incr << std::endl
	<< "vertex list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Vertex_const_iterator viter = dcel.vertexes_begin();
	viter != dcel.vertexes_end();
	++viter ) {
      ostr << brief( *viter ) << std::endl;
   }
   ostr << decr 
	<< "halfedge list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Halfedge_const_iterator hiter = dcel.halfedges_begin();
	hiter != dcel.halfedges_end();
	++hiter ) {
      ostr << brief( *hiter ) << std::endl;
   }
   ostr << decr 
	<< "facet list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Facet_const_iterator fiter = dcel.facets_begin();
	fiter != dcel.facets_end();
	++fiter ) {
      ostr << brief( *fiter ) << std::endl;
   }
   ostr << decr 
	<< decr << "}" << std::endl;
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const verbose_type<Dcel<Vb,Hb,Fb>::Vertex> &bvertex ) {
   const Dcel<Vb,Hb,Fb>::Vertex& vertex = bvertex.m_base;
   ostr << mem_ref(&vertex) << "[" << dgd 
	<< (const Dcel<Vb,Hb,Fb>::Vertex::Parent&)vertex
	<< "," << dgd << mem_ref(vertex.halfedge()) << "]";
   return ostr;
}


template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const verbose_type<Dcel<Vb,Hb,Fb>::Halfedge>& bhe ) {
   const Dcel<Vb,Hb,Fb>::Halfedge& he = bhe.m_base;
   ostr << mem_ref(&he) << "(";
   if( he.opposite() == NULL || he.opposite()->vertex() == NULL ) 
      ostr << "null";
   else
      ostr << mem_ref(he.opposite()->vertex()) 
	   << "[" << dgd << brief(*he.opposite()->vertex()) << "]";
   ostr << "->";
   if( he.vertex() == NULL ) 
      ostr << "null";
   else
      ostr << mem_ref(he.vertex())
	   << "[" << dgd << brief(*he.vertex()) << "]";
   ostr << ", facet:" << dgd << mem_ref(he.facet())
	<< ", opposite:" << dgd << mem_ref(he.opposite())
	<< ", next:" << dgd << mem_ref(he.next()) 
	<< ", prev:" << dgd << mem_ref(he.prev()) << ")";
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const verbose_type<Dcel<Vb,Hb,Fb>::Facet> &bf ) { 
   const Dcel<Vb,Hb,Fb>::Facet& f = bf.m_base;
   const Dcel<Vb,Hb,Fb>::Halfedge *he;

   ostr << mem_ref(&f) << "{" << incr << std::endl;
   he = f.halfedge();
   do {
      ostr << verbose(*he) << std::endl;
      ostr << verbose(*he->opposite()) << std::endl;
      he = he->next();
   } while( he != NULL && he != f.halfedge() );
   ostr << decr << "}";
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const verbose_type<Dcel<Vb,Hb,Fb> > &bdcel ) {
   const Dcel<Vb,Hb,Fb>& dcel = bdcel.m_base;
   ostr << "{" << dgd << incr << std::endl
	<< "vertex list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Vertex_const_iterator viter = dcel.vertexes_begin();
	viter != dcel.vertexes_end();
	++viter ) {
      ostr << verbose( *viter ) << std::endl;
   }
   ostr << decr 
	<< "halfedge list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Halfedge_const_iterator hiter = dcel.halfedges_begin();
	hiter != dcel.halfedges_end();
	++hiter ) {
      ostr << verbose( *hiter ) << std::endl;
   }
   ostr << decr 
	<< "facet list: " << dgd << incr << std::endl;
   for( Dcel<Vb,Hb,Fb>::Facet_const_iterator fiter = dcel.facets_begin();
	fiter != dcel.facets_end();
	++fiter ) {
      ostr << verbose( *fiter ) << std::endl;
   }
   ostr << decr 
	<< decr << "}" << std::endl;
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const Dcel<Vb,Hb,Fb>::Vertex& vertex ) {
    ostr << verbose(vertex);
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const Dcel<Vb,Hb,Fb>::Halfedge& he ) {
   ostr << verbose(he);
   return ostr;
}

template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const Dcel<Vb,Hb,Fb>::Facet &f ) { 
   ostr << verbose(f);
   return ostr;   
}


template <class Vb, class Hb, class Fb>
channel &operator << ( channel &ostr, 
		       const Dcel<Vb,Hb,Fb>& dcel ) {
   ostr << verbose(dcel);
   return ostr;
}

template <class T>
channel &operator << ( channel &ostr, const verbose_type<T*> &vf ) { 
   if( vf.m_base != NULL ) 
      ostr << verbose(*vf.m_base);
   else
      ostr << "NULL";
   return ostr;
}

template <class T>
channel &operator << ( channel &ostr, const brief_type<T*> &vf ) { 
   if( vf.m_base != NULL ) 
      ostr << brief(*vf.m_base);
   else
      ostr << "NULL";
   return ostr;
}


}; // end of namespace trace
}; // end of namespace nmm
}; // end of namespace scooter

#endif /* _dcel_trace_h_ */

//
// dcel_trace.h -- end of file
//

