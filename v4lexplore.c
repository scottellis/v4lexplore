#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <error.h>
#include <linux/videodev2.h>


struct defs_to_strings {
	unsigned long id;
	char idstr[32];
};

static struct defs_to_strings querycap_ids[] = {
	{ V4L2_CAP_VIDEO_CAPTURE, "V4L2_CAP_VIDEO_CAPTURE" },
	{ V4L2_CAP_VIDEO_OUTPUT, "V4L2_CAP_VIDEO_OUTPUT" },
	{ V4L2_CAP_VIDEO_OVERLAY, "V4L2_CAP_VIDEO_OVERLAY" },
	{ V4L2_CAP_VBI_CAPTURE, "V4L2_CAP_VBI_CAPTURE" }, { V4L2_CAP_VBI_OUTPUT, "V4L2_CAP_VBI_OUTPUT" },
	{ V4L2_CAP_SLICED_VBI_CAPTURE, "V4L2_CAP_SLICED_VBI_CAPTURE" },
	{ V4L2_CAP_SLICED_VBI_OUTPUT, "V4L2_CAP_SLICED_VBI_OUTPUT" },
	{ V4L2_CAP_RDS_CAPTURE, "V4L2_CAP_RDS_CAPTURE" },
	{ V4L2_CAP_VIDEO_OUTPUT_OVERLAY, "V4L2_CAP_VIDEO_OUTPUT_OVERLAY" },
	{ V4L2_CAP_HW_FREQ_SEEK, "V4L2_CAP_HW_FREQ_SEEK" },
	{ V4L2_CAP_TUNER, "V4L2_CAP_TUNER" },
	{ V4L2_CAP_AUDIO, "V4L2_CAP_AUDIO" },
	{ V4L2_CAP_RADIO, "V4L2_CAP_RADIO" },
	{ V4L2_CAP_READWRITE, "V4L2_CAP_READWRITE" },
	{ V4L2_CAP_ASYNCIO, "V4L2_CAP_ASYNCIO" },
	{ V4L2_CAP_STREAMING, "V4L2_CAP_STREAMING" },
	{ 0, "" }
};

static struct defs_to_strings pixel_fmt_ids[] = {
	{ V4L2_PIX_FMT_BGR24, "V4L2_PIX_FMT_BGR24" },
	{ V4L2_PIX_FMT_RGB24, "V4L2_PIX_FMT_RGB24" },
	{ V4L2_PIX_FMT_BGR32, "V4L2_PIX_FMT_BGR32" },
	{ V4L2_PIX_FMT_RGB32, "V4L2_PIX_FMT_RGB32" },
	{ V4L2_PIX_FMT_GREY, "V4L2_PIX_FMT_GREY" },
	{ V4L2_PIX_FMT_YUYV, "V4L2_PIX_FMT_YUYV" },
	{ V4L2_PIX_FMT_YVYU, "V4L2_PIX_FMT_YVYU" },
	{ V4L2_PIX_FMT_UYVY, "V4L2_PIX_FMT_UYVY" },
	{ V4L2_PIX_FMT_VYUY, "V4L2_PIX_FMT_VYUY" },
	{ V4L2_PIX_FMT_MJPEG, "V4L2_PIX_FMT_MJPEG" },
	{ V4L2_PIX_FMT_JPEG, "V4L2_PIX_FMT_JPEG" },
	{ 0, "" }
};



int vidioc_querycap(int fh);
int vidioc_enum_input(int fh);
int vidioc_enum_output(int fh);
int vidioc_enum_fmt(int fh);
int vidioc_queryctrl(int fh);

void fourcc_to_char(unsigned int cc, char *str);

void usage(char *argv_0)
{
	printf("Usage: %s [-d <device>] [properties]\n", argv_0);
 	printf("  -d <device>: defaults to /dev/video0\n");
	printf("  The options are numbers which query different v4l2\n");
	printf("  properties for the device as described below.\n");
	printf("  The default behavior is to query all properties.\n");
	printf("  Options:\n");
	printf("  -1: vidioc_querycap\n");
	printf("  -2: vidioc_enum_fmt\n");
	printf("  -3: vidioc_enum_framesizes\n");
	printf("  -4: vidioc_queryctrl\n"); 
	printf("\n");	

	exit(1);
}

int main(int argc, char **argv)
{
	int fh, opt;
	int prop[4];
	char device[32];

	memset(device, 0, sizeof(device));
	memset(prop, 0, sizeof(prop));

	while ((opt = getopt(argc, argv, "-d:1234h")) != -1) {
		switch (opt) {
		case 'd':
			strncpy(device, optarg, sizeof(device) - 1);
			break;
	
		case '1':
		case '2':
		case '3':
		case '4':
			prop[opt - '1'] = 1;
			break;

		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	if (strlen(device) == 0)
		strcpy(device, "/dev/video0");

	if ((fh = open(device, O_RDWR)) < 0) {
		perror("open");
		return 1;
	}

	if (prop[0])
		vidioc_querycap(fh);

	if (prop[1])
		vidioc_enum_fmt(fh);

	if (prop[3])
		vidioc_queryctrl(fh);

	printf("\n");

	close(fh);

	return 0;
}

int vidioc_queryctrl(int fh)
{
	int i;
	struct v4l2_queryctrl qc;

	printf("\n=== VIDIOC_QUERYCTRL ===\n\n");

	i = 0;
	memset(&qc, 0, sizeof(qc));
	qc.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	while (0 == ioctl(fh, VIDIOC_QUERYCTRL, &qc)) {
		printf("Control: %d\n", i++);
		
		if (qc.id >= V4L2_CID_BASE && qc.id < V4L2_CID_LASTP1)
			printf("\tid: V4L2_CID_BASE + %d\n", 
				qc.id - V4L2_CID_BASE);
		else if (qc.id >= V4L2_CID_CAMERA_CLASS_BASE && qc.id <= V4L2_CID_CAMERA_CLASS_BASE + 20)
			printf("\tid: V4L2_CID_CAMERA_CLASS_BASE + %d\n", 
				qc.id - V4L2_CID_CAMERA_CLASS_BASE);  
		else
			printf("\tid: 0x%08X\n", qc.id);

		if (strlen((char *)qc.name) > 0)
			printf("\tname: %s\n", (char *) qc.name);

		switch (qc.type) {
		case V4L2_CTRL_TYPE_INTEGER:
			printf("\ttype: integer\n");
			printf("\tminimum: %d\n", qc.minimum);
			printf("\tmaximum: %d\n", qc.maximum);
			printf("\tstep: %d\n", qc.step);
			printf("\tdefault: %d\n", qc.default_value);
			break;
		case V4L2_CTRL_TYPE_BOOLEAN:
			printf("\ttype: boolean\n");
			printf("\tdefault: %d\n", qc.default_value);
			break;			
		default:
			printf("\ttype: other: %d\n", (int) qc.type);
			printf("\tminimum: %d\n", qc.minimum);
			printf("\tmaximum: %d\n", qc.maximum);
			printf("\tstep: %d\n", qc.step);
			printf("\tdefault: %d\n", qc.default_value);
		}

		if (qc.flags)
			printf("\tflags: 0x%08X\n", qc.flags);
		
		qc.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}


	return 0;
}

int vidioc_enum_fmt(int fh)
{
	int i;
	struct v4l2_fmtdesc f;
	char buff[8];

	i = 0;

	printf("\n=== VIDIOC_ENUM_FMT (type = V4L2_BUF_TYPE_VIDEO_CAPTURE) ===\n\n");

	while (1) {
		memset(&f, 0, sizeof(f));
		f.index = i++;
		f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (ioctl(fh, VIDIOC_ENUM_FMT, &f) < 0)
			break;
	
		printf("Format %d:\n", i);

		if (f.flags & V4L2_FMT_FLAG_COMPRESSED)
			printf("\tflags: V4L2_FMT_FLAG_COMPRESSED\n");

		if (strlen((char *)f.description) > 0)
			printf("\tdescription: %s\n", f.description);

		fourcc_to_char(f.pixelformat, buff);
		printf("\tpixelformat: 0x%08X (%s)\n", f.pixelformat, buff);		

		for (i = 0; pixel_fmt_ids[i].id != 0; i++) {
			if (pixel_fmt_ids[i].id == f.pixelformat) {
				printf("\tdefinition: %s\n", pixel_fmt_ids[i].idstr);
				break;
			}
		}
	}
	
	return 0;
}

void fourcc_to_char(unsigned int cc, char *str)
{
	int i;

	for (i = 0; i < 4; i++) {
		str[i] = cc & 0x7f;
		cc >>= 8;
	}

	str[4] = 0;
}

int vidioc_querycap(int fh)
{
	int i;
	struct v4l2_capability cap;

	bzero(&cap, sizeof(cap));

	if (ioctl(fh, VIDIOC_QUERYCAP, &cap) < 0) {
		perror("ioctl(VIDIOC_QUERYCAP)");
		return -1;
	}

	printf("\n=== VIDIOC_QUERYCAP ===\n\n");

	if (strlen((char *)cap.driver) > 0)
		printf("driver: %s\n", cap.driver);

	if (strlen((char *)cap.card) > 0)
		printf("card: %s\n", cap.card);

	if (strlen((char *)cap.bus_info) > 0)
		printf("bus_info: %s\n", cap.bus_info);

	printf("version: %u\n", cap.version);

	printf("capabilites:\n");

	for (i = 0; querycap_ids[i].id != 0; i++) {
		if (cap.capabilities & querycap_ids[i].id)
			printf("\t%s\n", querycap_ids[i].idstr);
	}

	return 0;
}

int vidioc_enum_input(int fh)
{
	int i;
	struct v4l2_input input;

	i = 0;

	printf("\n=== VIDIOC_ENUMINPUT ===\n\n");

	while (1) {
		bzero(&input, sizeof(input));
		input.index = i++;

		if (ioctl(fh, VIDIOC_ENUMINPUT, &input) < 0)
			break;

		printf("Input %d\n", i);

		if (strlen((char *)input.name) > 0)
			printf("\tname: %s\n", (char *)input.name);

		if (input.type == V4L2_INPUT_TYPE_TUNER)
			printf("\ttype: V4L2_INPUT_TYPE_TUNER\n");
		else if (input.type == V4L2_INPUT_TYPE_CAMERA)
			printf("\ttype: V4L2_INPUT_TYPE_CAMERA\n");
		else
			printf("\ttype: unknown %u\n", input.type);

		printf("\tstatus: (not all checked)\n");

		if (input.status & V4L2_IN_ST_NO_POWER)
			printf("\t\tV4L2_IN_ST_NO_POWER\n");

		if (input.status & V4L2_IN_ST_NO_SIGNAL)
			printf("\t\tV4L2_IN_ST_NO_SIGNAL\n");

		if (input.status & V4L2_IN_ST_NO_COLOR)
			printf("\t\tV4L2_IN_ST_NO_COLOR\n");

		if (input.status & V4L2_IN_ST_HFLIP)
			printf("\t\tV4L2_IN_ST_HFLIP\n");

		if (input.status & V4L2_IN_ST_VFLIP)
			printf("\t\tV4L2_IN_ST_VFLIP\n");
	}

	return 0; 
}

int vidioc_enum_output(int fh)
{
	int i;
	struct v4l2_output output;

	i = 0;

	printf("\nResults of VIDIOC_ENUMOUTPUT:\n");

	while (1) {
		output.index = i++;
		
		if (ioctl(fh, VIDIOC_ENUMINPUT, &output) < 0)
			break;

		printf("output: %d\n", i);
		/*
		if (strlen((char *)output.name) > 0)
			printf("\tname: %s\n", (char *)output.name);

		if (output.type == V4L2_OUTPUT_TYPE_MODULATOR)
			printf("\ttype: V4L2_OUTPUT_TYPE_MODULATOR\n");
		else if (output.type == V4L2_OUTPUT_TYPE_ANALOG)
			printf("\ttype: V4L2_OUTPUT_TYPE_ANALOG\n");
		else if (output.type == V4L2_OUTPUT_TYPE_ANALOGVGAOVERLAY)
			printf("\ttype: V4L2_OUTPUT_TYPE_ANALOGVGAOVERLAY\n");
		else
			printf("\ttype: unknown %u\n", output.type);

		printf("\taudioset: 0x%08X\n", output.audioset);
		printf("\tmodulator: 0x%08X\n", output.modulator);
		std = 0xFFFFFFFF & output.std;
		printf("\tstd: 0x%08X\n", std);			
		*/
	}

	return 0; 
}

