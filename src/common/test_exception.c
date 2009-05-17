

#include <stdio.h>
#include <common/exception.h>

static void video_subsys1_cleanup(struct cleanup * str)
{
	printf("video subsys1 cleanup\n");
	return;
}

static void video_subsys2_cleanup(struct cleanup * str)
{
	printf("video subsys2 cleanup\n");
	return;
}

static struct cleanup video_subsys1_cleanup_str = {
	.function	= video_subsys1_cleanup,
};

static struct cleanup video_subsys2_cleanup_str = {
	.function	= video_subsys2_cleanup,
};

static void
video_subsys_1_init(bool_t add_cleanup)
{
	if (add_cleanup)
		make_cleanup(&video_subsys1_cleanup_str);
	printf("video_subsys_1_init\n");
	return;
}

static void
video_subsys_2_init(bool_t add_cleanup)
{
	if (add_cleanup)
		make_cleanup(&video_subsys2_cleanup_str);
	printf("video_subsys_2_init\n");
	return;
}

static void
video_init(bool_t add_cleanup)
{
	printf("In Video Init\n");
	video_subsys_1_init(add_cleanup);
	video_subsys_2_init(add_cleanup);

}

static void
video_reinit()
{
	printf("Video reinit\n");
	video_subsys2_cleanup(NULL);
	video_subsys1_cleanup(NULL);
	video_init(FALSE);
}

static void
snd_subsys1_cleanup(struct cleanup * str)
{
	printf("snd subsys1 cleanup\n");
	return;
}

static void
snd_subsys2_cleanup(struct cleanup * str)
{
	printf("snd subsys2 cleanup\n");
	return;
}

static struct cleanup snd_subsys1_cleanup_str = {
	.function	= snd_subsys1_cleanup,
};

static struct cleanup snd_subsys2_cleanup_str = {
	.function	= snd_subsys2_cleanup,
};


static void
snd_subsys1_init(bool_t add_cleanup)
{
	if (add_cleanup)
		make_cleanup(&snd_subsys1_cleanup_str);
	printf("snd subsys1 init\n");
}

static void
snd_subsys2_init(bool_t add_cleanup)
{
	if (add_cleanup)
		make_cleanup(&snd_subsys2_cleanup_str);
	printf("snd subsys2 init\n");
}

static void
snd_init(bool_t add_cleanup)
{
	snd_subsys1_init(add_cleanup);
	snd_subsys2_init(add_cleanup);
	return;
}

static void
snd_reinit()
{
	printf("snd reinit\n");
	snd_subsys2_cleanup(NULL);
	snd_subsys1_cleanup(NULL);
	snd_init(FALSE);
}

static void
video_frame_cleanup(struct cleanup * str)
{
	printf("video frame cleanup\n");
}

static struct cleanup video_frame_cleanup_str = {
	.function	= video_frame_cleanup,
};


static void
video_frame(int i)
{
	volatile struct exception exp;
	static int reruned = 0;
	static int reinited = 0;
	static int sysreinited = 0;
	static int sysreruned = 0;
entry:
	TRY_CATCH(exp, MASK_SUBSYS_ALL) {
		make_cleanup(&video_frame_cleanup_str);
		printf("video frame %d called\n", i);
		if ((i == 1) && (reruned == 0)) {
			reruned = 1;
			THROW(EXCEPTION_SUBSYS_RERUN, "video frame 1 need rerun");
		}

		if ((i == 2) && (reinited == 0)) {
			reinited = 1;
			THROW(EXCEPTION_SUBSYS_REINIT, "video frame 2 need video sys reinit");
		}

		if (i == 3) {
			THROW(EXCEPTION_SUBSYS_SKIPFRAME, "video frame 3 need video sys skip");
		}

		if (i == 4) {
			THROW(EXCEPTION_SYS_SKIPFRAME, "video frame 4 need whole sys skip");
		}

		if ((i == 5) && (sysreinited == 0)) {
			sysreinited = 1;
			THROW(EXCEPTION_SYS_REINIT, "video frame 5 need whole sys reinit");
		}

		if ((i == 6) && (sysreruned == 0)) {
			sysreruned = 1;
			THROW(EXCEPTION_SYS_RERUN, "video frame 6 need whole sys rerun");
		}

		if ((i == 7)) {
			THROW(EXCEPTION_USER_QUIT, "video frame 7 user quit");
		}

		printf("video frame %d finished\n", i);
	}
	END_TRY;
	switch (exp.level) {
		case EXCEPTION_NO_ERROR:
			return;
		case EXCEPTION_SUBSYS_RERUN:
			printf("frame %d rerun, reason:%s\n", i, exp.message);
			goto entry;
			break;
		case EXCEPTION_SUBSYS_REINIT:
			printf("frame %d reinit, reason:%s\n", i, exp.message);
			video_reinit();
			goto entry;
			break;
		case EXCEPTION_SUBSYS_SKIPFRAME:
			printf("frame %d skipped, reason:%s\n", i, exp.message);
			break;
		default:
			INTERNAL_ERROR(SYSTEM, "???");
			break;
	}
}

static void
snd_frame(int i)
{
	printf("snd frame %d called\n", i);
	printf("snd frame %d finished\n", i);
}



static void
run()
{
	int i;
	volatile struct exception exp;

	i = 0;
entry:
	TRY_CATCH(exp, MASK_SYS_ALL) {
		for (; i < 10; i++) {
			video_frame(i);
			snd_frame(i);
		}
	}
	END_TRY;
	switch (exp.level) {
		case EXCEPTION_NO_ERROR:
			printf("No error! WOW!!!\n");
			break;
		case EXCEPTION_SYS_RERUN:
			printf("sys frame %d sysrerun, reason:%s\n", i, exp.message);
			i -= 1;
			goto entry;
			break;
		case EXCEPTION_SYS_SKIPFRAME:
			printf("sys frame %d skipped, reason: %s\n", i, exp.message);
			i ++;
			goto entry;
			break;
		case EXCEPTION_SYS_REINIT:
			printf("sys frame %d call sysreinit, reason: %s\n", i, exp.message);
			video_reinit();
			snd_reinit();
			goto entry;
			break;
		default:
			INTERNAL_ERROR(SYSTEM, "????");
			break;
	}
}



int main()
{
	DEBUG_INIT(NULL);
	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		video_init(TRUE);
		snd_init(TRUE);
		run();
	}
	END_TRY;


	switch (exp.level) {
		case (EXCEPTION_NO_ERROR):
			printf("No error!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			printf("User quit! msg=%s\n", exp.message);
			break;
		default:
			printf("Error!!!\n");
			break;
	}

	


	do_cleanup();
	return 0;
}

