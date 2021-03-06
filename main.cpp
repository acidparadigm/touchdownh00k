#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "log.hpp"
#include "remote.hpp"
#include "hack.hpp"
#include "netvar.hpp"

using namespace std;

bool shouldGlow = true;
bool shouldLog = false;

// This isn't called or used because it's for debugging, use it if you want.
void printAllClasses() {
    for (size_t i = 0; i < netvar::GetAllClasses().size(); i++) {
        cout << netvar::GetAllClasses()[i].name << endl;

        for (size_t p = 0; p < netvar::GetAllClasses()[i].props.size(); p++) {
            cout << "\t" << netvar::GetAllClasses()[i].props[p].name << " = " << std::hex <<
            netvar::GetAllClasses()[i].props[p].offset << endl;
        }
    }
}

int main() {
    if (getuid() != 0) {
        cout << "run this as root fgt" << endl;
        return 0;
    }

    cout << "..:t0uchdown h00k:.. 0x2016 :: isoteric" << endl;

	if(shouldLog) {
		log::init();
		log::put("Hack has loaded...");
	}

	Display* dpy = XOpenDisplay(0);
	Window root = DefaultRootWindow(dpy);
	XEvent ev;

	int keycode = XKeysymToKeycode(dpy, XK_X);
	unsigned int modifiers = ControlMask | ShiftMask;

	XGrabKey(dpy, keycode, modifiers, root, false,
				GrabModeAsync, GrabModeAsync);
	XSelectInput(dpy, root, KeyPressMask);
	
    remote::Handle csgo;
    
    while(true) {

		while (true) {
		    if (remote::FindProcessByName("csgo_linux", &csgo)) {
		        break;
		    }

		    usleep(500);
		}

		cout << "CSGO Process Located [" << csgo.GetPath() << "][" << csgo.GetPid() << "]" << endl;

		remote::MapModuleMemoryRegion client;

		client.start = 0;

		while (client.start == 0) {
		    if (!csgo.IsRunning()) {
		        cout << "Exited game before client could be located, terminating" << endl;
		        return 0;
		    }

		    csgo.ParseMaps();

		    for (auto region : csgo.regions) {
		        if (region.filename.compare("client_client.so") == 0 && region.executable) {
		            cout << "client_client.so: [" << std::hex << region.start << "][" << std::hex << region.end << "][" <<
		            region.pathname << "]" << endl;
		            client = region;
		            break;
		        }
		    }

		    usleep(500);
		}

		cout << "GlowObject Size: " << std::hex << sizeof(hack::GlowObjectDefinition_t) << endl;

		cout << "Found client_client.so [" << std::hex << client.start << "]" << endl;
		client.client_start = client.start;

		void* foundGlowPointerCall = client.find(csgo,
		                                         "\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6",
		                                         "x????xxxxx");

	//Old Sig Pre 11/10/15
	//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6\x34
	//x????xxxxxx
	//New Sig as of 11/10/15
	//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6\x38
	//x????xxxxxx
	//11/10/15 Sig reduction, we don't need the size
	//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6
	//x????xxxxx

		cout << "Glow Pointer Call Reference: " << std::hex << foundGlowPointerCall <<
		" | Offset: " << (unsigned long) foundGlowPointerCall - client.start << endl;

		if (!foundGlowPointerCall) {
		    cout << "Unable to find glow pointer call reference" << endl;
		    return 0;
		}

		unsigned long call = csgo.GetCallAddress(foundGlowPointerCall);

		if (!call) {
		    cout << "Unable to read glow pointer call reference address" << endl;
		    return 0;
		}

		cout << "Glow function address: " << std::hex << call << endl;
		cout << "Glow function address offset: " << std::hex << call - client.start << endl;

		unsigned long addressOfGlowPointer = call;

		if (!csgo.Read((void*) (call + 0x11), &addressOfGlowPointer, sizeof(unsigned long))) {
		    cout << "Unable to read address of glow pointer" << endl;
		    return 0;
		}

		cout << "Glow Array: " << std::hex << addressOfGlowPointer  << endl;

		while (!netvar::Cache(csgo, client)) {
		    if (!csgo.IsRunning()) {
		        cout << "Exited game before netvars could be cached, terminating" << endl;
		        return 0;
		    }

		    usleep(50000);
		}

		cout << "Cached " << std::dec << netvar::GetAllClasses().size() << " networked classes" << endl;

		while (csgo.IsRunning()) {
			while (XPending(dpy) > 0) {
				XNextEvent(dpy, &ev);
				switch (ev.type) {
					case KeyPress:
						cout << "Toggling glow..." << endl;
						XUngrabKey(dpy, keycode, modifiers, root);
						shouldGlow = !shouldGlow;
						break;
					default:
						break;
				}

				XGrabKey(dpy, keycode, modifiers, root, false,
							GrabModeAsync, GrabModeAsync);
				XSelectInput(dpy, root, KeyPressMask);
			}

			if (shouldGlow)
			    hack::Glow(&csgo, &client, addressOfGlowPointer);

		    usleep(2000);
		}

		cout << "Game ended." << endl << endl;
		
	}

    return 0;
}
