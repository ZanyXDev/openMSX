// $Id$

#include <cassert>
#include "IDEHD.hh"
#include "File.hh"
#include "FileContext.hh"
#include "xmlx.hh"
#include "EventDistributor.hh"
#include "LedEvent.hh"


namespace openmsx {

byte IDEHD::identifyBlock[512] = {
	0x5a,0x0c,0xba,0x09,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x31,0x20,0x00,0x00,0x00,0x01,0x04,0x00,0x31,0x56,
	0x30,0x2e,0x20,0x20,0x20,0x20,0x50,0x4f,0x4e,0x45,0x4d,0x20,0x58,0x53,0x48,0x20,
	0x52,0x41,0x20,0x44,0x49,0x44,0x4b,0x53,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x10,0x80,
	0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x02,0x00,0x02,0x03,0x00,0xba,0x09,0x10,0x00,
	0x3f,0x00,0x60,0x4c,0x26,0x00,0x00,0x00,0xe0,0x53,0x26,0x00,0x07,0x00,0x07,0x04,
	0x03,0x00,0x78,0x00,0x78,0x00,0xf0,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


IDEHD::IDEHD(const XMLElement& config, const EmuTime& /*time*/)
{
	buffer = new byte[512 * 256];

	const string& filename = config.getChildData("filename");
	file.reset(new File(config.getFileContext().resolveCreate(filename), CREATE));
	
	unsigned wantedSize = config.getChildDataAsInt("size");	// in MB
	wantedSize *= 1024 * 1024;
	unsigned fileSize = file->getSize();
	if (wantedSize > fileSize) {
		// for safety only enlarge file
		file->truncate(wantedSize);
	}
	
	totalSectors = wantedSize / 512;
	word heads = 16;
	word sectors = 32;
	word cylinders = totalSectors / (heads * sectors);
	identifyBlock[0x02] = cylinders & 0xFF;
	identifyBlock[0x03] = cylinders / 0x100;
	identifyBlock[0x06] = heads & 0xFF;
	identifyBlock[0x07] = heads / 0x100;
	identifyBlock[0x0C] = sectors & 0xFF;
	identifyBlock[0x0D] = sectors / 0x100;

	transferRead = transferWrite = false;
}

IDEHD::~IDEHD()
{
	delete[] buffer;
}


void IDEHD::reset(const EmuTime& /*time*/)
{
	errorReg = 0x01;
	sectorCountReg = 0x01;
	sectorNumReg = 0x01;
	cylinderLowReg = 0x00;
	cylinderHighReg = 0x00;
	devHeadReg = 0x00;
	statusReg = 0x50;	// DRDY DSC
	featureReg = 0x00;
	setTransferRead(false);
	setTransferWrite(false);
}

byte IDEHD::readReg(nibble reg, const EmuTime& /*time*/)
{
	switch (reg) {
	case 1:	// error register
		return errorReg;
	
	case 2:	// sector count register
		return sectorCountReg;
		
	case 3:	// sector number register
		return sectorNumReg;
	
	case 4:	// cyclinder low register
		return cylinderLowReg;
		
	case 5:	// cyclinder high register
		return cylinderHighReg;
		
	case 6:	// device/head register
		// DEV bit is handled by IDE interface
		return devHeadReg;
		
	case 7:	// status register
		return statusReg;

	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 15:// not used
		return 0x7F;
		
	case 0:	// data register, converted to readData by IDE interface
	case 14:// alternate status reg, converted to read from normal
		// status register by IDE interface
	default:
		assert(false);
		return 0x7F;	// avoid warning
	}
}

void IDEHD::writeReg(nibble reg, byte value, const EmuTime& /*time*/)
{
	switch (reg) {
	case 1:	// feature register
		featureReg = value;
		break;
	
	case 2:	// sector count register
		sectorCountReg = value;
		break;
		
	case 3:	// sector number register
		sectorNumReg = value;
		break;
	
	case 4:	// cyclinder low register
		cylinderLowReg = value;
		break;
		
	case 5:	// cyclinder high register
		cylinderHighReg = value;
		break;
		
	case 6:	// device/head register
		// DEV bit is handled by IDE interface
		devHeadReg = value;
		break;
		
	case 7:	// command register
		executeCommand(value);
		break;

	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 15:// not used
	case 14:// device control register, handled by IDE interface
		// do nothing
		break;
		
	case 0:	// data register, converted to readData by IDE interface
	default:
		assert(false);
		break;
	}
}


word IDEHD::readData(const EmuTime& /*time*/)
{
	if (!transferRead) {
		// no read in progress
		return 0x7F7F;
	}
	word result = *(transferPntr++);
	if (--transferCount == 0) {
		// everything read
		setTransferRead(false);
		statusReg &= ~0x08;	// DRQ
		PRT_DEBUG("IDEHD: read sector done");
	}
	return result;
}

void IDEHD::writeData(word value, const EmuTime& /*time*/)
{
	if (!transferWrite) {
		// no write in progress
		return;
	}
	*(transferPntr++) = value;
	transferCount--;
	if ((transferCount & 255) == 0) {
		try {
			file->seek(512 * transferSectorNumber);
			file->write(buffer, 512);
		} catch (FileException &e) {
			setError(0x44);
			setTransferWrite(false);
		}
		PRT_DEBUG("IDEHD: written sector " << transferSectorNumber);
		transferSectorNumber++;
		transferPntr = (word*)buffer;
	}
	if (transferCount == 0) {
		// everything written
		setTransferWrite(false);
		statusReg &= ~0x08;	// DRQ
	}
}


void IDEHD::setError(byte error)
{
	errorReg = error;
	statusReg |= 0x01;	// ERR
	statusReg &= ~0x08;	// DRQ
	setTransferWrite(false);
	setTransferRead(false);
}

unsigned IDEHD::getSectorNumber() const
{
	return sectorNumReg | (cylinderLowReg << 8) |
		(cylinderHighReg << 16) | ((devHeadReg & 0x0F) << 24);
}

unsigned IDEHD::getNumSectors() const
{
	return (sectorCountReg == 0) ? 256 : sectorCountReg;
}

void IDEHD::executeCommand(byte cmd)
{
	statusReg &= ~0x09;	// reset DRQ ERR
	setTransferRead(false);
	setTransferWrite(false);
	switch (cmd) {
	case 0xEF: // Set Feature
		if (featureReg != 0x03) {
			// not "Set Transfer Mode"
			setError(0x04);
		}
		break;

	case 0xEC: // ATA Identify Device
		transferCount = 512/2;
		transferPntr = (word*)(&identifyBlock);
		setTransferRead(true);
		statusReg |= 0x08;	// DRQ
		break;

	case 0x91: // Initialize Device Parameters
		// ignore command
		break;

	case 0x30: { // Write Sector
		int sectorNumber = getSectorNumber();
		int numSectors = getNumSectors();
		PRT_DEBUG("IDEHD: write sector " << sectorNumber << " " << numSectors);
		if ((sectorNumber + numSectors) > totalSectors) {
			setError(0x14);
			break;
		}
		transferSectorNumber = sectorNumber;
		transferCount = 512/2 * numSectors;
		transferPntr = (word*)buffer;
		setTransferWrite(true);
		statusReg |= 0x08;	// DRQ
		break;
	}
	case 0x20: { // Read Sector
		int sectorNumber = getSectorNumber();
		int numSectors = getNumSectors();
		PRT_DEBUG("IDEHD: read sector " << sectorNumber << " " << numSectors);
		if ((sectorNumber + numSectors) > totalSectors) {
			setError(0x14);
			break;
		}
		try {
			file->seek(512 * sectorNumber);
			file->read(buffer, 512 * numSectors);
		} catch (FileException &e) {
			setError(0x44);
			break;
		}
		transferCount = 512/2 * numSectors;
		transferPntr = (word*)buffer;
		setTransferRead(true);
		statusReg |= 0x08;	// DRQ
		break;
	}
	default: // all others
		setError(0x04);
	}
}

void IDEHD::setTransferRead(bool status)
{
	if (status != transferRead) {
		transferRead = status;
		if (!transferWrite) {
			// (this is a bit of a hack!)
			EventDistributor::instance().distributeEvent(
				new LedEvent(LedEvent::FDD, transferRead));
		}
	}
}

void IDEHD::setTransferWrite(bool status)
{
	if (status != transferWrite) {
		transferWrite = status;
		if (!transferRead) {
			// (this is a bit of a hack!)
			EventDistributor::instance().distributeEvent(
				new LedEvent(LedEvent::FDD, transferWrite));
		}
	}
}

} // namespace openmsx
