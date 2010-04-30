/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2008 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _DLVHASHIMPL_H
#define _DLVHASHIMPL_H

// implementation of DLVertex Hash; to be included after DLDag definition

inline BipolarPointer
dlVHashTable :: locate ( const HashLeaf& leaf, const DLVertex& v ) const
{
	for ( HashLeaf::const_iterator p = leaf.begin(), p_end = leaf.end(); p != p_end; ++p )
		if ( v == host[*p] )
			return *p;

	return bpINVALID;
}

inline BipolarPointer
dlVHashTable :: locate ( const DLVertex& v ) const
{
	HashTable::const_iterator p = Table.find(hash(v));
	return p == Table.end() ? bpINVALID : locate ( p->second, v );
}

inline void
dlVHashTable :: addElement ( BipolarPointer pos )
{
	insert ( Table[hash(host[pos])], pos );
}

#endif

