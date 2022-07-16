/***********************************************************************/
/*                                                                     */
/*  SYSCALLS.C:  System Calls                                          */
/*  most of this is from newlib-lpc and a Keil-demo                    */
/*                                                                     */
/*  These are "reentrant functions" as needed by                       */
/*  the WinARM-newlib-config, see newlib-manual.                       */
/*  Collected and modified by Martin Thomas                            */
/*                                                                     */
/***********************************************************************/

/* adapted for the SAM7 "serial.h" mthomas 10/2005 */

#include <stdlib.h>
#include <reent.h>
#include <sys/stat.h>
#include "DDSinc.h"

extern int global_index;
extern unsigned char global_tab[NB_CHARS];

//#include "ADuC7026.h"

//int uart0_kbhit()
//{return(1);}

//int uart0_getc()
//{while(!(0x01==(COMSTA0 & 0x01))) {}
// return (COMRX);
//}

//int uart0_putc(int ch)
//{
// while(!(0x020==(COMSTA0 & 0x020))) {}
// return (COMTX = ch);
//}

/* int uart0_putc(int ch) {return(1);} */ // NE MARCHE PAS : PLANTE !

_ssize_t _read_r(__attribute__((unused))struct _reent *r, __attribute__((unused))int file, void *ptr, size_t len)
{
	char c;
	unsigned int  i;
	unsigned char *p;
	
	p = (unsigned char*)ptr;
	
	for (i = 0; i < len; i++) {
		while ( global_index == 0) {} //  !uart0_kbhit() ) ;
		c = global_tab[0];global_index=0; // (char) uart0_getc();
		if (c == 0x0D) {
			*p='\0';
			break;
		}
		*p++ = c;
		jmf_putchar(c,NULL,0,0);
	}
	return len - i;
}


_ssize_t _write_r (__attribute__((unused)) struct _reent *r, __attribute__((unused))int file, const void *ptr, size_t len)
{
	unsigned int i;
	const unsigned char *p;
	
	p = (const unsigned char*) ptr;
	
	for (i = 0; i < len; i++) {
		if (*p == '\n' ) jmf_putchar('\r',NULL,0,0);
		jmf_putchar(*p++,NULL,0,0);
	}
	
	return len;
}


int _close_r(  
    __attribute__((unused))struct _reent *r, 
    __attribute__((unused))int file)
{
	return 0;
}


_off_t _lseek_r(
    __attribute__((unused))struct _reent *r, 
    __attribute__((unused))int file, 
    __attribute__((unused))_off_t ptr, 
    __attribute__((unused))int dir)
{
	return (_off_t)0;	/*  Always indicate we are at file beginning.  */
}


int _fstat_r(
    __attribute__((unused))struct _reent *r, 
    __attribute__((unused))int file, 
    struct stat *st)
{
	/*  Always set as character device.				*/
	st->st_mode = S_IFCHR;
	/* assigned to strong type with implicit 	*/
	/* signed/unsigned conversion.  Required by 	*/
	/* newlib.					*/

	return 0;
}


// inutile : jmfriedt
int _isatty(int file); // avoid warning 

int _isatty(__attribute__((unused))int file)
{
	return 1;
}


#if 0
static void _exit (int n) {
label:  goto label; /* endless loop */
}
#endif 


/* "malloc clue function" from newlib-lpc/Keil-Demo/"generic" */

/**** Locally used variables. ****/
// mt: "cleaner": extern char* end;
extern char end[];              /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr;		/* Points to current end of the heap.	*/

/************************** _sbrk_r *************************************
 * Support function. Adjusts end of heap to provide more memory to
 * memory allocator. Simple and dumb with no sanity checks.

 *  struct _reent *r -- re-entrancy structure, used by newlib to
 *                      support multiple threads of operation.
 *  ptrdiff_t nbytes -- number of bytes to add.
 *                      Returns pointer to start of new heap area.
 *
 *  Note:  This implementation is not thread safe (despite taking a
 *         _reent structure as a parameter).
 *         Since _s_r is not used in the current implementation, 
 *         the following messages must be suppressed.
 */
void * _sbrk_r(
    __attribute__((unused))struct _reent *_s_r, 
    ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error  */

	if (!heap_ptr) {	/*  Initialize if first time through.  */
		heap_ptr = end;
	}
	base = heap_ptr;	/*  Point to end of heap.  */
	heap_ptr += nbytes;	/*  Increase heap.  */
	
	return base;		/*  Return pointer to start of new heap area.*/
}
