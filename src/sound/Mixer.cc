// $Id$

#include <cassert>
#include <algorithm>
#include "Mixer.hh"
#include "MSXCPU.hh"
#include "RealTime.hh"
#include "SoundDevice.hh"
#include "MSXConfig.hh"
#include "Config.hh"
#include "CliCommOutput.hh"
#include "InfoCommand.hh"
#include "Scheduler.hh"

namespace openmsx {

Mixer::Mixer()
	: muteCount(0),
	  cpu(MSXCPU::instance()),
	  realTime(RealTime::instance()),
	  msxConfig(MSXConfig::instance()),
	  output(CliCommOutput::instance()),
	  infoCommand(InfoCommand::instance()),
	  muteSetting("mute", "(un)mute the emulation sound", false),
	  pauseSetting(Scheduler::instance().getPauseSetting()),
	  soundDeviceInfo(*this)
{
	prevLeft = prevOutLeft = 0;
	prevRight = prevOutRight = 0;
#ifdef DEBUG_MIXER
	nbClipped = 0;
#endif

	// default values
	int freq = 22050;
	int samples = 512;
	try {
		Config* config = msxConfig.getConfigById("Mixer");
		freq = config->getParameterAsInt("frequency", freq);
		samples = config->getParameterAsInt("samples", samples);
	} catch (ConfigException &e) {
		// no Mixer section
	}

	SDL_AudioSpec desired;
	desired.freq     = freq;
	desired.samples  = samples;
	desired.channels = 2;			// stereo
#ifdef WORDS_BIGENDIAN
	desired.format   = AUDIO_S16MSB;
#else
	desired.format   = AUDIO_S16LSB;
#endif

	desired.callback = audioCallbackHelper;	// must be a static method
	desired.userdata = this;
	if (SDL_OpenAudio(&desired, &audioSpec) < 0) {
		output.printWarning(
		        string("Couldn't open audio : ") + SDL_GetError());
		init = false;
	} else {
		init = true;
		mixBuffer = new short[audioSpec.size / sizeof(short)];
		reInit();
	}
	infoCommand.registerTopic("sounddevice", &soundDeviceInfo);
	muteSetting.addListener(this);
	pauseSetting.addListener(this);
}

Mixer::~Mixer()
{
	pauseSetting.removeListener(this);
	muteSetting.removeListener(this);
	infoCommand.unregisterTopic("sounddevice", &soundDeviceInfo);
	if (init) {
		SDL_CloseAudio();
		delete[] mixBuffer;
	}
}

Mixer& Mixer::instance()
{
	static Mixer oneInstance;
	return oneInstance;
}

int Mixer::registerSound(SoundDevice* device, short volume, ChannelMode mode)
{
	if (!init) {
		// sound disabled
		return 512;	// return a save value
	}
	
	const string& name = device->getName();
	SoundDeviceInfo info;
	info.volumeSetting = new IntegerSetting(name + "_volume",
			"the volume of this sound chip", volume, 0, 32767);

	map<string, ChannelMode> modeMap;
	// once we're stereo, stay stereo. Once mono, stay mono.
	// we could also choose not to offer any modeSetting in case we have
	// a stereo mode initially. You can't query the mode then, though.
	if (mode == STEREO) {
		modeMap["stereo"] = STEREO;
	} else {
		modeMap["mono"] = MONO;
		modeMap["left"] = MONO_LEFT;
		modeMap["right"] = MONO_RIGHT;
	}
	info.modeSetting = new EnumSetting<ChannelMode>(name + "_mode", "the channel mode of this sound chip", mode, modeMap);
	
	info.mode = mode;
	info.modeSetting->addListener(this);
	info.volumeSetting->addListener(this);
	infos[device] = info;

	lock();
	if (buffers.size() == 0) {
		SDL_PauseAudio(0);	// unpause when first dev registers
	}
	buffers.push_back(NULL);	// make room for one more
	devices[mode].push_back(device);
	device->setSampleRate(audioSpec.freq);
	device->setVolume(volume);
	unlock();

	return audioSpec.samples;
}

void Mixer::unregisterSound(SoundDevice *device)
{
	if (!init) {
		return;
	}
	map<SoundDevice*, SoundDeviceInfo>::iterator it=
		infos.find(device);
	if (it == infos.end()) {
		return;
	}
	lock();
	ChannelMode mode = it->second.mode;
	vector<SoundDevice*> &dev = devices[mode];
	dev.erase(remove(dev.begin(), dev.end(), device), dev.end());
	buffers.pop_back();
	it->second.volumeSetting->removeListener(this);
	delete it->second.volumeSetting;
	it->second.modeSetting->removeListener(this);
	delete it->second.modeSetting;
	
	if (buffers.size() == 0) {
		SDL_PauseAudio(1);	// pause when last dev unregisters
	}
	unlock();
}


void Mixer::audioCallbackHelper (void *userdata, Uint8 *strm, int len)
{
	// len ignored
	short *stream = (short*)strm;
	((Mixer*)userdata)->audioCallback(stream);
}

void Mixer::audioCallback(short* stream)
{
	updtStrm(samplesLeft);
	memcpy(stream, mixBuffer, audioSpec.size);
	reInit();
}

void Mixer::reInit()
{
	samplesLeft = audioSpec.samples;
	offset = 0;
	prevTime = cpu.getCurrentTimeUnsafe(); // !! can be one instruction off
}

void Mixer::updateStream(const EmuTime &time)
{
	if (!init) return;

	if (prevTime < time) {
		float duration = realTime.getRealDuration(prevTime, time);
		//PRT_DEBUG("Mix: update, duration " << duration << "s");
		assert(duration >= 0);
		prevTime = time;
		lock();
		updtStrm((int)(audioSpec.freq * duration));
		unlock();
	}
}
void Mixer::updtStrm(int samples)
{
	if (samples > samplesLeft)
		samples = samplesLeft;
	if (samples == 0)
		return;
	//PRT_DEBUG("Mix: Generate " << samples << " samples");

	int modeOffset[NB_MODES];
	int unmuted = 0;
	for (int mode = 0; mode < NB_MODES; mode++) {
		modeOffset[mode] = unmuted;
		for (vector<SoundDevice*>::const_iterator i =
		           devices[mode].begin();
		     i != devices[mode].end();
		     ++i) {
			int *buf = (*i)->updateBuffer(samples);
			if (buf != NULL) {
				buffers[unmuted++] = buf;
			}
		}
	}
	for (int j = 0; j < samples; ++j) {
		int buf = 0;
		int both = 0;
		while (buf < modeOffset[MONO+1]) {
			both  += buffers[buf++][j];
		}
		int left = both;
		while (buf < modeOffset[MONO_LEFT+1]) {
			left  += buffers[buf++][j];
		}
		int right = both;
		while (buf < modeOffset[MONO_RIGHT+1]) {
			right += buffers[buf++][j];
		}
		while (buf < unmuted) {
			left  += buffers[buf]  [2*j+0];
			right += buffers[buf++][2*j+1];
		}
		
		// 1st order IIR filter
		//   y(n) = a0 * x(n) + a1 * x(n-1) + b1 * y(n-1)
		// high-pass filter:
		//   a0 =  (1 + x) / 2      x = exp(-2 * pi * f / fsamp)
		//   a1 = -(1 + x) / 2      f = cut-off freq
		//   b1 =   x               fsamp = sample freq
		// take x = 1022/1024
		//   44100Hz --> cutt-off freq = 14Hz
		//   22050Hz                     7Hz
		int tmp;
		tmp = 1023 * left;
		left = (tmp - prevLeft + 1022 * prevOutLeft) >> 10;
		prevLeft = tmp;
		prevOutLeft = left;

		tmp = 1023 * right;
		right = (tmp - prevRight + 1022 * prevOutRight) >> 10;
		prevRight = tmp;
		prevOutRight = right;
 
		// clip
		#ifdef DEBUG_MIXER
		if ((left  > 32767) || (left  < -32768) ||
		    (right > 32767) || (right < -32768)) {
			nbClipped++;
			PRT_DEBUG("Mixer: clipped " << nbClipped);
		}
		#endif
		if      (left  > 32767)  left  =  32767;
		else if (left  < -32768) left  = -32768;
		if      (right > 32767)  right =  32767;
		else if (right < -32768) right = -32768;

		mixBuffer[offset++] = (short)left;
		mixBuffer[offset++] = (short)right;
	}
	samplesLeft -= samples;
}

void Mixer::lock()
{
	if (!init) return;

	SDL_LockAudio();
}

void Mixer::unlock()
{
	if (!init) return;

	SDL_UnlockAudio();
}

void Mixer::mute()
{
	muteHelper(++muteCount);
}
void Mixer::unmute()
{
	assert(muteCount);
	muteHelper(--muteCount);
}
void Mixer::muteHelper(int muteCount)
{
	if (!init) return;

	if (buffers.size() == 0)
		return;
	SDL_PauseAudio(muteCount);
}


void Mixer::update(const SettingLeafNode *setting) throw()
{
	if (setting == &muteSetting) {
		if (muteSetting.getValue()) {
			mute();
		} else {
			unmute();
		}
	} else if (setting == &pauseSetting) {
		if (pauseSetting.getValue()) {
			mute();
		} else {
			unmute();
		}
	} else {
		const EnumSetting<ChannelMode>* s = dynamic_cast <const EnumSetting<ChannelMode>* >(setting);
		if (s!=NULL) {
			map<SoundDevice*, SoundDeviceInfo>::iterator it = infos.begin();
			while (it != infos.end() && it->second.modeSetting != setting) ++it;
			assert (it!=infos.end());
			// it->first is the SoundDevice we need
			SoundDeviceInfo &info = it->second;
			lock();
			ChannelMode oldmode = info.mode;
			info.mode = info.modeSetting->getValue();
			vector<SoundDevice*> &dev = devices[oldmode];
			dev.erase(remove(dev.begin(), dev.end(), it->first), dev.end());
			devices[info.mode].push_back(it->first);
			unlock();
		} else {
			const IntegerSetting* t = dynamic_cast <const IntegerSetting*>(setting);
			if (t != NULL) {
				map<SoundDevice*, SoundDeviceInfo>::iterator it = infos.begin();
				while (it != infos.end() && it->second.volumeSetting != setting) ++it;
				assert (it!=infos.end());
				// it->first is the SoundDevice we need
				it->first->setVolume(it->second.volumeSetting->getValue()); 
			} else {
				assert(false);
			}
		}
	}
}


// Sound device info

SoundDevice* Mixer::getSoundDevice(const string& name)
{
	for (map<SoundDevice*, SoundDeviceInfo>::const_iterator it =
	       infos.begin(); it != infos.end(); ++it) {
		if (it->first->getName() == name) {
			return it->first;
		}
	}
	return NULL;
}

Mixer::SoundDeviceInfoTopic::SoundDeviceInfoTopic(Mixer& parent_)
	: parent(parent_)
{
}

string Mixer::SoundDeviceInfoTopic::execute(const vector<string> &tokens) const
	throw(CommandException)
{
	string result;
	switch (tokens.size()) {
	case 2:
		for (map<SoundDevice*, SoundDeviceInfo>::const_iterator it =
		       parent.infos.begin(); it != parent.infos.end(); ++it) {
			result += it->first->getName() + '\n';
		}
		break;
	case 3: {
		SoundDevice* device = parent.getSoundDevice(tokens[2]);
		if (!device) {
			throw CommandException("Unknown sound device");
		}
		result = device->getDescription();
		break;
	}
	default:
		throw CommandException("Too many parameters");
	}
	return result;
}

string Mixer::SoundDeviceInfoTopic::help(const vector<string> &tokens) const
	throw()
{
	return "Shows a list of available sound devices.\n";
}

void Mixer::SoundDeviceInfoTopic::tabCompletion(vector<string>& tokens) const
	throw()
{
	if (tokens.size() == 3) {
		set<string> devices;
		for (map<SoundDevice*, SoundDeviceInfo>::const_iterator it =
		       parent.infos.begin(); it != parent.infos.end(); ++it) {
			devices.insert(it->first->getName());
		}
		CommandController::completeString(tokens, devices);
	}
}

} // namespace openmsx
