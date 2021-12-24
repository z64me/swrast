#ifndef SWR_INPUT_H
#define SWR_INPUT_H

struct swr_input {
	float delta_time_sec;
	struct {
		struct {
			int x;
			int y;
		} pos;
		struct {
			int left;
			int middle;
			int right;
			int leftclick;
		} button;
		struct {
			int dx;
			int dy;
		} wheel;
	} mouse;
	struct {
		char w, a, s, d, lshift, lctrl, space;
	} key;
};

#endif /* SWR_INPUT_H */

