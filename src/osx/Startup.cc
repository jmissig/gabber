#include <Carbon/Carbon.h>
#include <gconfmm/init.h>

namespace Gabber {

void StartupPlatform()
{
//    EventRecord dummyVariable;
//    WaitNextEvent(everyEvent, &dummyVariable, 0, NULL);
//	IBNibRef nibref;
//	OSStatus err;
//	err = CreateNibReference(CFSTR("Gabber"), &nibref);
//	if ( err == kIBCarbonRuntimeCantFindNibFile)
//	{
//		SInt16 sel;
//		StandardAlert(kAlertStopAlert, "test", NULL, NULL, &sel);
//	}
//	
//	MenuRef ref;
//	err = CreateMenuFromNib(nibref, CFSTR("DockMenu"), &ref);
//	if (err == kIBCarbonRuntimeCantFindObject || err == kIBCarbonRuntimeObjectNotOfRequestedType)
//	{
//		SInt16 sel;
//		StandardAlert(kAlertStopAlert, "Error getting menu", NULL, NULL, &sel);
//	}
//	SetApplicationDockTileMenu(ref);

    Gnome::Conf::init();
	
	
}

void ShutdownPlatform()
{
}

};
