#include <common/exception.h>


static void
cleanup_outmost1(struct cleanup * clup)
{
	remove_cleanup(clup);
	FORCE(SYSTEM, "cleanup outmost 1\n");
	return;
}

static void
cleanup_outmost2(struct cleanup * clup)
{
	remove_cleanup(clup);
	FORCE(SYSTEM, "cleanup outmost 2\n");
	return;
}

static void
cleanup_inner1(struct cleanup * clup)
{
	remove_cleanup(clup);
	FORCE(SYSTEM, "cleanup inner 1\n");
	return;
}

static void
cleanup_inner2(struct cleanup * clup)
{
	remove_cleanup(clup);
	FORCE(SYSTEM, "cleanup inner 2\n");
	FATAL(SYSTEM, "Trigger another exception in cleanup code\n");
	THROW(EXCEPTION_FATAL, "Fatal in cleanup code");
	return;
}

static void
cleanup_inner3(struct cleanup * clup)
{
	remove_cleanup(clup);
	FORCE(SYSTEM, "cleanup inner 3\n");
	return;
}




static struct cleanup cu_outmost1 = {
	.function = cleanup_outmost1,
};

static struct cleanup cu_outmost2 = {
	.function = cleanup_outmost2,
};

static struct cleanup cu_inner1 = {
	.function = cleanup_inner1,
};

static struct cleanup cu_inner2 = {
	.function = cleanup_inner2,
};

static struct cleanup cu_inner3 = {
	.function = cleanup_inner3,
};


/* test throw exception while doing cleanup */
int main()
{

	DEBUG_INIT(NULL);

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		FORCE(SYSTEM, "In outmost try-catch block\n");
		make_cleanup(&cu_outmost1);
		make_cleanup(&cu_outmost2);
		volatile struct exception exp;
		TRY_CATCH(exp, MASK_NONFATAL) {
			FORCE(SYSTEM, "In inner try-catch block\n");

			make_cleanup(&cu_inner1);
			make_cleanup(&cu_inner2);
			make_cleanup(&cu_inner3);

			/* trigger an exception here */
			FATAL(SYSTEM, "Trigger an exception here\n");
			THROW(EXCEPTION_FATAL, "Trigger an exception here");
		}
		END_TRY;
		FORCE(SYSTEM, "Finish inner try-catch block\n");
	}
	CATCH(exp) {
		default:
			print_exception(FATAL, SYSTEM, exp);
	}

	do_cleanup();
	return 0;
}

