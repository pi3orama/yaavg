#include <stdio.h>
#include <common/exception.h>

int
funx(int a)
{
	printf("[-] call funx a=%d\n",a);
	if (a == 17) {
		printf("[-] funcx throw exception\n");
		THROW(EXCEPTION_RENDER_ERROR, "Quit!!");
	}
	printf("[-] funx return a+10\n");
	return a+10;
}

int
main()
{
	DEBUG_INIT(NULL);
	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		NOTHROW(funx, 1);
		NOTHROW(funx, 17);
		printf("[+] funx return %d\n", NOTHROW_RET(funx, 100, 2));

#define XXX	do {printf("[+] cleanup\n");} while(0)
		TRY_CLEANUP(funx, XXX, 3);
#define FFF	do {printf("[+] finally\n");} while(0)
		TRY_FINALLY(funx, FFF, 3);
		TRY_CLEANUP(funx, XXX, 17);
#undef XXX

	}
	CATCH(exp) {
		case (EXCEPTION_NO_ERROR):
			printf("No error!\n");
			break;
		default:
			printf("error with msg %s, val=%d\n",
					exp.message, exp.val);
	}
	return 0;
}


