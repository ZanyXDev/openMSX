// $Id$

#include "msxconfig.hh"
#include "devicefactory.hh"
#include "MSXSimple64KB.hh"
#include "MSXRom16KB.hh"
#include "MSXPPI.hh"
#include "MSXTMS9928a.hh"
#include "MSXE6Timer.hh"
#include "MSXCPU.hh"
#include "MSXPSG.hh"
#include "MSXKanji.hh"
#include "MSXMemoryMapper.hh"
#include "MSXMapperIO.hh"
#include <assert.h>

MSXDevice *deviceFactory::create(MSXConfig::Device *conf) {
	MSXDevice *device = 0;
	if ( conf->getType().compare("MotherBoard") == 0 ) {
		// if 0 then strings are equal
		device = MSXMotherBoard::instance();
	}
	if ( conf->getType().compare("Rom16KB") == 0 ) {
		device = new MSXRom16KB();
	}
	if ( conf->getType().compare("Simple64KB") == 0 ) {
		device = new MSXSimple64KB();
	}
	if ( conf->getType().compare("PPI") == 0 ) {
		device = MSXPPI::instance();
	}
	if ( conf->getType().compare("TMS9928a") == 0 ) {
		device = new MSXTMS9928a();
	}
	if ( conf->getType().compare("E6Timer") == 0 ) {
		device = new MSXE6Timer();
	}
	if ( conf->getType().compare("CPU") == 0 ) {
		device = MSXCPU::instance();
	}
	if ( conf->getType().compare("PSG") == 0 ) {
		device = new MSXPSG();
	}
	if ( conf->getType().compare("Kanji") == 0 ) {
		device = new MSXKanji();
	}
	if ( conf->getType().compare("MemoryMapper") == 0 ) {
		device = new MSXMemoryMapper();
	}
	if ( conf->getType().compare("MapperIO") == 0 ) {
		device = MSXMapperIO::instance();
	}

	assert (device != 0);
	device->setConfigDevice(conf);
	return device;
}

