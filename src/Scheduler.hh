// $Id$

#ifndef __SCHEDULER_HH__
#define __SCHEDULER_HH__

#include "emutime.hh"
#include "HotKey.hh"
#include <SDL/SDL.h>
#include <set>
#include <string>


class Schedulable
{
	public:
		virtual void executeUntilEmuTime(const Emutime &time) = 0;
		virtual const std::string &getName();
	protected:
		static const std::string defaultName;
};


class SynchronizationPoint
{
	public:
		SynchronizationPoint (const Emutime &time, Schedulable &dev) : timeStamp(time), device(dev) {}
		const Emutime &getTime() const { return timeStamp; }
		Schedulable &getDevice() const { return device; }
		bool operator< (const SynchronizationPoint &n) const { return getTime() < n.getTime(); }
	private: 
		const Emutime timeStamp;	// copy of original timestamp
		Schedulable &device;		// alias
};

class Scheduler : public EventListener, public HotKeyListener
{
	public:
		virtual ~Scheduler();
		static Scheduler *instance();
		void setSyncPoint(const Emutime &timestamp, Schedulable &activedevice);
		void scheduleEmulation();
		void stopScheduling();
		// EventListener
		void signalEvent(SDL_Event &event);
		// HotKeyListener
		void signalHotKey(SDLKey key);
		
	private:
		Scheduler();
		const SynchronizationPoint &getFirstSP();
		void removeFirstSP();
		
		static Scheduler *oneInstance;
		std::set<SynchronizationPoint> scheduleList;

		static const Emutime infinity;

		bool paused;
		bool runningScheduler;
		SDL_mutex *pauseMutex;
};

#endif
