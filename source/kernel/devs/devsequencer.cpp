/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "boxedwine.h"

#include "../../io/fsvirtualopennode.h"

class DevSequencer : public FsVirtualOpenNode {
public:
    DevSequencer(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {}

	// From FsOpenNode
    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
};

FsOpenNode* openDevSequencer(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevSequencer(node, flags);
}
/*
struct synth_info {
		char	name[30];
		int	device;
		int	synth_type;
#define SYNTH_TYPE_FM			0
#define SYNTH_TYPE_SAMPLE		1
#define SYNTH_TYPE_MIDI			2	// Midi interface 

		int	synth_subtype;
#define FM_TYPE_ADLIB			0x00
#define FM_TYPE_OPL3			0x01
#define MIDI_TYPE_MPU401		0x401

#define SAMPLE_TYPE_BASIC		0x10
#define SAMPLE_TYPE_GUS			SAMPLE_TYPE_BASIC
#define SAMPLE_TYPE_WAVEFRONT           0x11

		int	perc_mode;	// No longer supported 
		int	nr_voices;
		int	nr_drums;	// Obsolete field 
		int	instr_bank_size;
		unsigned int	capabilities;	
#define SYNTH_CAP_PERCMODE		0x00000001 // No longer used
#define SYNTH_CAP_OPL3			0x00000002 // Set if OPL3 supported
#define SYNTH_CAP_INPUT			0x00000004 // Input (MIDI) device 
		int	dummies[19];	// Reserve space 
	};

struct midi_info		// OBSOLETE 
{
  char name[30];
  int device;			// 0-N. INITIALIZE BEFORE CALLING 
  unsigned int capabilities;	// To be defined later 
  int dev_type;
  int dummies[18];		// Reserve space 
};
*/
U32 DevSequencer::ioctl(KThread* thread, U32 request) {
    //U32 len = (request >> 16) & 0x3FFF;
    CPU* cpu = thread->cpu;
	KMemory* memory = thread->memory;

    //bool read = (request & 0x40000000) != 0;
    bool write = (request & 0x80000000) != 0;

    switch (request & 0xFFFF) {     
    case 0x5100: // LINUX_SNDCTL_SEQ_RESET	
        return 0;
    case 0x5102: // LINUX_SNDCTL_SYNTH_INFO	        
        return 0;
    case 0x510a: // LINUX_SNDCTL_SEQ_NRSYNTHS
        if (write)
            memory->writed(IOCTL_ARG1, 0);
        return 0;
    case 0x510b: // LINUX_SNDCTL_SEQ_NRMIDIS
        if (write)
			memory->writed(IOCTL_ARG1, 1);
        return 0;
    case 0x510c: // LINUX_SNDCTL_MIDI_INFO	
        memory->strcpy(IOCTL_ARG1, "Boxedwine MIDI");
		memory->writed(IOCTL_ARG1+36, 0); // capabilities
		memory->writed(IOCTL_ARG1+40, 0); // dev_type
        return 0;
    }
    return -K_ENODEV;
}

U32 DevSequencer::readNative(U8* buffer, U32 len) {
    return 0;
}

#ifdef BOXEDWINE_MIDI

#define SYSEX_SIZE 1024
#define RAWBUF	1024

U8 MIDI_evt_len[256] = {
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x00
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x10
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x20
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x30
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x40
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x50
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x60
  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  // 0x70

  3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,  // 0x80
  3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,  // 0x90
  3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,  // 0xa0
  3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,  // 0xb0

  2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,  // 0xc0
  2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,  // 0xd0

  3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3,  // 0xe0

  0,2,3,2, 0,0,1,0, 1,0,1,1, 1,0,1,0   // 0xf0
};

static struct {
	U32 status;
	U32 cmd_len;
	U32 cmd_pos;
	U8 cmd_buf[8];
	U8 rt_buf[8];
	struct {
		U8 buf[SYSEX_SIZE];
		U8 used;
	} sysex;
	bool available;
} midi;

void MIDI_RawOutByte(U8 data) {
    if (KSystem::soundEnabled) {
	    /* Test for a realtime MIDI message */
	    if (data>=0xf8) {
		    midi.rt_buf[0]=data;
		    PlayMsg(midi.rt_buf);
		    return;
	    }	 
	    /* Test for a active sysex tranfer */
	    if (midi.status==0xf0) {
		    if (!(data&0x80)) { 
			    if (midi.sysex.used<(SYSEX_SIZE-1)) midi.sysex.buf[midi.sysex.used++]=data;
			    return;
		    } else {
			    midi.sysex.buf[midi.sysex.used++]=0xf7;
			    PlaySysex(midi.sysex.buf,midi.sysex.used);
		    }
	    }
	    if (data&0x80) {
		    midi.status=data;
		    midi.cmd_pos=0;
		    midi.cmd_len=MIDI_evt_len[data];
		    if (midi.status==0xf0) {
			    midi.sysex.buf[0]=0xf0;
			    midi.sysex.used=1;
		    }
	    }
	    if (midi.cmd_len) {
		    midi.cmd_buf[midi.cmd_pos++]=data;
		    if (midi.cmd_pos >= midi.cmd_len) {
			    PlayMsg(midi.cmd_buf);
			    midi.cmd_pos=1;		//Use Running status
		    }
	    }
    }
}
#endif

U32 DevSequencer::writeNative(U8* buffer, U32 len) {
#ifdef BOXEDWINE_MIDI
    for (U32 i=0;i<len;i+=4) {
        if (buffer[i]==5) {
            MIDI_RawOutByte(buffer[i+1]);
        } else {
            klog("Unhandled midi msg: %X", buffer[i]);
        }
    }
#endif
    return len;
}
