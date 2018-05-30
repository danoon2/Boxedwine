#ifndef __OSS_H__
#define __OSS_H__

#define AFMT_QUERY               0x00000000      /* Return current fmt */
#define AFMT_MU_LAW              0x00000001
#define AFMT_A_LAW               0x00000002
#define AFMT_IMA_ADPCM           0x00000004
#define AFMT_U8                  0x00000008
#define AFMT_S16_LE              0x00000010      /* Little endian signed 16*/
#define AFMT_S16_BE              0x00000020      /* Big endian signed 16 */
#define AFMT_S8                  0x00000040
#define AFMT_U16_LE              0x00000080      /* Little endian U16 */
#define AFMT_U16_BE              0x00000100      /* Big endian U16 */
#define AFMT_MPEG                0x00000200      /* MPEG (2) audio */

#define PCM_ENABLE_INPUT         0x00000001
#define PCM_ENABLE_OUTPUT        0x00000002

#define DSP_CAP_REVISION         0x000000ff      /* Bits for revision level (0 to 255) */
#define DSP_CAP_DUPLEX           0x00000100      /* Full duplex record/playback */
#define DSP_CAP_REALTIME         0x00000200      /* Real time capability */
#define DSP_CAP_BATCH            0x00000400      /* Device has some kind of */
                                                 /* internal buffers which may */
                                                 /* cause some delays and */
                                                 /* decrease precision of timing */
#define DSP_CAP_COPROC           0x00000800      /* Has a coprocessor */
                                                 /* Sometimes it's a DSP */
                                                 /* but usually not */
#define DSP_CAP_TRIGGER          0x00001000      /* Supports SETTRIGGER */
#define DSP_CAP_MMAP             0x00002000      /* Supports mmap() */
#define DSP_CAP_MULTI            0x00004000      /* support multiple open */
#define DSP_CAP_BIND             0x00008000      /* channel binding to front/rear/cneter/lfe */

typedef struct oss_sysinfo
{
  char product[32];		/* For example OSS/Free, OSS/Linux or OSS/Solaris */
  char version[32];		/* For example 4.0a */
  int versionnum;		/* See OSS_GETVERSION */
  char options[128];		/* Reserved */

  int numaudios;		/* # of audio/dsp devices */
  int openedaudio[8];		/* Bit mask telling which audio devices are busy */

  int numsynths;		/* # of availavle synth devices */
  int nummidis;			/* # of available MIDI ports */
  int numtimers;		/* # of available timer devices */
  int nummixers;		/* # of mixer devices */

  int openedmidi[8];		/* Bit mask telling which midi devices are busy */
  int numcards;			/* Number of sound cards in the system */
  int numaudioengines;		/* Number of audio engines in the system */
  char license[16];		/* For example "GPL" or "CDDL" */
  char revision_info[256];	/* For internal use */
  int filler[172];		/* For future expansion */
} oss_sysinfo;

#define OSS_LONGNAME_SIZE	64
#define OSS_LABEL_SIZE		16
#define OSS_DEVNODE_SIZE	32
#define OSS_DEVNAME_SIZE	64
#define OSS_CMD_SIZE		64
#define OSS_ID_SIZE		16
#define OSS_HANDLE_SIZE		32
typedef char oss_longname_t[OSS_LONGNAME_SIZE];
typedef char oss_label_t[OSS_LABEL_SIZE];
typedef char oss_devnode_t[OSS_DEVNODE_SIZE];
typedef char oss_devname_t[OSS_DEVNAME_SIZE];
typedef char oss_cmd_t[OSS_CMD_SIZE];
typedef char oss_id_t[OSS_ID_SIZE];
typedef char oss_handle_t[OSS_HANDLE_SIZE];

typedef struct oss_audioinfo
{
  int dev;			/* Audio device number */
  oss_devname_t name;
  int busy;			/* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
  int pid;
  int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
  int iformats, oformats;
  int magic;			/* Reserved for internal use */
  oss_cmd_t cmd;		/* Command using the device (if known) */
  int card_number;
  int port_number;
  int mixer_dev;
  int legacy_device;		/* Obsolete field. Replaced by devnode */
  int enabled;			/* 1=enabled, 0=device not ready at this moment */
  int flags;			/* For internal use only - no practical meaning */
  int min_rate, max_rate;	/* Sample rate limits */
  int min_channels, max_channels;	/* Number of channels supported */
  int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
  int rate_source;
  oss_handle_t handle;
#define OSS_MAX_SAMPLE_RATES	20	/* Cannot be changed  */
  unsigned int nrates, rates[OSS_MAX_SAMPLE_RATES];	/* Please read the manual before using these */
  oss_longname_t song_name;	/* Song name (if given) */
  oss_label_t label;		/* Device label (if given) */
  int latency;			/* In usecs, -1=unknown */
  oss_devnode_t devnode;	/* Device special file name (absolute path) */
  int next_play_engine;		/* Read the documentation for more info */
  int next_rec_engine;		/* Read the documentation for more info */
  int filler[184];
} oss_audioinfo;

#	define PCM_CAP_REVISION		0x000000ff	/* Bits for revision level (0 to 255) */
#	define PCM_CAP_DUPLEX		0x00000100	/* Full duplex record/playback */
#	define PCM_CAP_REALTIME		0x00000200	/* Not in use */
#	define PCM_CAP_BATCH		0x00000400	/* Device has some kind of */
							/* internal buffers which may */
							/* cause some delays and */
							/* decrease precision of timing */
#	define PCM_CAP_COPROC		0x00000800	/* Has a coprocessor */
							/* Sometimes it's a DSP */
							/* but usually not */
#	define PCM_CAP_TRIGGER		0x00001000	/* Supports SETTRIGGER */
#	define PCM_CAP_MMAP		0x00002000	/* Supports mmap() */
#	define PCM_CAP_MULTI		0x00004000	/* Supports multiple open */
#	define PCM_CAP_BIND		0x00008000	/* Supports binding to front/rear/center/lfe */
#   	define PCM_CAP_INPUT		0x00010000	/* Supports recording */
#   	define PCM_CAP_OUTPUT		0x00020000	/* Supports playback */
#	define PCM_CAP_VIRTUAL		0x00040000	/* Virtual device */
/* 0x00040000 and 0x00080000 reserved for future use */

/* Analog/digital control capabilities */
#	define PCM_CAP_ANALOGOUT	0x00100000
#	define PCM_CAP_ANALOGIN		0x00200000
#	define PCM_CAP_DIGITALOUT	0x00400000
#	define PCM_CAP_DIGITALIN	0x00800000
#	define PCM_CAP_ADMASK		0x00f00000

/* Audio data formats (Note! U8=8 and S16_LE=16 for compatibility) */
#define SNDCTL_DSP_GETFMTS		__SIOR ('P',11, int)	/* Returns a mask */
#define SNDCTL_DSP_SETFMT		__SIOWR('P',5, int)	/* Selects ONE fmt */
#	define AFMT_QUERY	0x00000000	/* Return current fmt */
#	define AFMT_MU_LAW	0x00000001
#	define AFMT_A_LAW	0x00000002
#	define AFMT_IMA_ADPCM	0x00000004
#	define AFMT_U8		0x00000008
#	define AFMT_S16_LE	0x00000010	/* Little endian signed 16 */
#	define AFMT_S16_BE	0x00000020	/* Big endian signed 16 */
#	define AFMT_S8		0x00000040
#	define AFMT_U16_LE	0x00000080	/* Little endian U16 */
#	define AFMT_U16_BE	0x00000100	/* Big endian U16 */
#	define AFMT_MPEG	0x00000200	/* MPEG (2) audio */

/* AC3 _compressed_ bitstreams (See Programmer's Guide for details). */
#	define AFMT_AC3		0x00000400
/* Ogg Vorbis _compressed_ bit streams */
#	define AFMT_VORBIS	0x00000800

/* 32 bit formats (MSB aligned) formats */
#	define AFMT_S32_LE	0x00001000
#	define AFMT_S32_BE	0x00002000

/* Reserved for _native_ endian double precision IEEE floating point */
#	define AFMT_FLOAT	0x00004000

/* 24 bit formats (LSB aligned in 32 bit word) formats */
#	define AFMT_S24_LE	0x00008000
#	define AFMT_S24_BE	0x00010000

#endif