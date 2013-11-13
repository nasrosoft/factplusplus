/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <string>
#include <iostream>

class SaveLoadManager
{
protected:
		/// name of S/L dir
	std::string dirname;
		/// file name
	std::string filename;
		/// input stream pointer
	std::istream* ip;
		/// output stream pointer
	std::ostream* op;

public:
		/// init c'tor: remember the S/L name
	SaveLoadManager ( const std::string& name ) : dirname(name), ip(NULL), op(NULL) { filename = name+".fpp.state"; }
		/// empty d'tor
	~SaveLoadManager ( void ) {}

	// context information

		/// @return true if there is some S/L content
	bool existsContent ( void ) const;
		/// clear all the content corresponding to the manager
	void clearContent ( void ) const;

	// set up stream

		/// prepare stream according to INPUT value
	void prepare ( bool input );
		/// get an input stream
	std::istream& i ( void ) { return *ip; }
		/// get an output stream
	std::ostream& o ( void ) { return *op; }
}; // SaveLoadManager

#endif
