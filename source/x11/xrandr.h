#ifndef __XRANDR_H__
#define __XRANDR_H__

/* Event selection bits */
#define RRScreenChangeNotifyMask  (1L << 0)
/* V1.2 additions */
#define RRCrtcChangeNotifyMask	    (1L << 1)
#define RROutputChangeNotifyMask    (1L << 2)
#define RROutputPropertyNotifyMask  (1L << 3)
/* V1.4 additions */
#define RRProviderChangeNotifyMask   (1L << 4)
#define RRProviderPropertyNotifyMask (1L << 5)
#define RRResourceChangeNotifyMask   (1L << 6)
/* V1.6 additions */
#define RRLeaseNotifyMask            (1L << 7)

/* Event codes */
#define RRScreenChangeNotify	0
/* V1.2 additions */
#define RRNotify		    1
/* RRNotify Subcodes */
#define  RRNotify_CrtcChange	    0
#define  RRNotify_OutputChange	    1
#define  RRNotify_OutputProperty    2
#define  RRNotify_ProviderChange    3
#define  RRNotify_ProviderProperty  4
#define  RRNotify_ResourceChange    5
/* V1.6 additions */
#define  RRNotify_Lease             6
/* used in the rotation field; rotation and reflection in 0.1 proto. */
#define RR_Rotate_0		1
#define RR_Rotate_90		2
#define RR_Rotate_180		4
#define RR_Rotate_270		8

typedef XID RROutput;
typedef XID RRCrtc;
typedef XID RRMode;
typedef XID RRProvider;

struct XRRScreenSize {
    S32	width, height;
    S32	mwidth, mheight;
};

static_assert(sizeof(XRRScreenSize) == 16, "emulation expects sizeof(XRRScreenSize) to be 16");

typedef U32 XRRModeFlags;

struct XRRModeInfo {
    RRMode		id;
    U32	width;
    U32	height;
    U32	dotClock;
    U32	hSyncStart;
    U32	hSyncEnd;
    U32	hTotal;
    U32	hSkew;
    U32	vSyncStart;
    U32	vSyncEnd;
    U32	vTotal;
    U32 name; // char*
    U32	nameLength;
    XRRModeFlags	modeFlags;
};

struct XRRScreenResources {
    Time	timestamp;
    Time	configTimestamp;
    S32		ncrtc;
    U32     crtcs; // RRCrtc*
    S32		noutput;
    U32     outputs; // RROutput*
    S32		nmode;
    U32     modes; // XRRModeInfo*
};

class XrrData {
public:
    U32 sizesAddress = 0; // XRRScreenSize*
    U32 sizesCount = 0;
    U32 ratesAddress = 0;
};

#define RRSetConfigSuccess		0
#define RRSetConfigInvalidConfigTime	1
#define RRSetConfigInvalidTime		2
#define RRSetConfigFailed		3

U32 XrrGetSizes(KThread* thread, const DisplayDataPtr& data, U32 screen, U32 countAddress);
U32 XrrConfigCurrentConfiguration(KThread* thread, const DisplayDataPtr& data, U32 rotationAddress);
U32 XrrConfigCurrentRate();
U32 XrrRates(KThread* thread, const DisplayDataPtr& data, U32 screen, U32 sizeIndex, U32 rateCountAddress);
bool XrrGetSize(KThread* thread, const DisplayDataPtr& displayData, U32 sizeIndex, U32& cx, U32& cy);

#endif