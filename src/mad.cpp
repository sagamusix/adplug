/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999, 2000, 2001, 2002 Simon Peter, <dn.tlp@gmx.net>, et al.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  mad.cpp - MAD loader by Riven the Mage <riven@ok.ru>
*/

#include "mad.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CmadLoader::factory(Copl *newopl)
{
  CmadLoader *p = new CmadLoader(newopl);
  return p;
}

bool CmadLoader::load(istream &f, const char *filename)
{
	const unsigned char conv_inst[10] = { 2,1,10,9,4,3,6,5,8,7 };

	unsigned int i,j,k,t=0;

	// 'MAD+' - signed ?
	char id[4];
	f.read(id,4);
	if (strncmp(id,"MAD+",4))
		return false;

	// load instruments
	f.read((char *)instruments,9*sizeof(mad_instrument));

	f.ignore(1);

	// data for Protracker
	length = f.get();
	nop = f.get();
	timer = f.get();

	// init CmodPlayer
	realloc_instruments(9);
	realloc_order(length);
	realloc_patterns(nop,32,9);

	init_trackord();

	// load tracks
	for (i=0;i<nop;i++)
	{
		for (k=0;k<32;k++)
		{
			for (j=0;j<9;j++)
			{
				t = i*9+j;

				// read event
				unsigned char event = f.get();

				// convert event
				if (event < 0x61)
					tracks[t][k].note = event;
				if (event == 0xFF) // 0xFF: Release note
					tracks[t][k].command = 8;
				if (event == 0xFE) // 0xFE: Pattern Break
					tracks[t][k].command = 13;
			}
		}
	}

	// load order
	f.read((char *)order,length);

	// convert instruments
	for (i=0;i<9;i++)
		for (j=0;j<10;j++)
			inst[i].data[conv_inst[j]] = instruments[i].data[j];

	// convert order
	for(i=0;i<length;i++)
		order[i]--;

	// data for Protracker
	restartpos = 0;
	initspeed = 1;

	rewind(0);

	return true;
}

void CmadLoader::rewind(unsigned int subsong)
{
	CmodPlayer::rewind(subsong);

	// default instruments
	for (int i=0;i<9;i++)
	{
		channel[i].inst = i;

		channel[i].vol1 = 63 - (inst[i].data[10] & 63);
		channel[i].vol2 = 63 - (inst[i].data[9] & 63);
	}
}

float CmadLoader::getrefresh()
{
	return (float)timer;
}

std::string CmadLoader::gettype()
{
	return std::string("Mlat Adlib Tracker");
}

std::string CmadLoader::getinstrument(unsigned int n)
{
	return std::string(instruments[n].name,8);
}

unsigned int CmadLoader::getinstruments()
{
	return 9;
}
