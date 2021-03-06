#ifndef GETTOKH
#define GETTOKH


/* gettok.h -- header file for gettok

        THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
        "UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
        AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
        CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
        PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
        RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
        S.M. Orlow
        Systex,Inc.
        Beltsville, MD 20705
        301-474-0111
        June, 1986

Contractor:
        K. E. Gorlen
        Bg. 12A, Rm. 2017
        Computer Systems Laboratory
        Division of Computer Research and Technology
        National Institutes of Health
        Bethesda, Maryland 20892
        Phone: (301) 496-5363
        uucp: uunet!nih-csl!kgorlen
        Internet: kgorlen@alw.nih.gov

log:    GETTOK.H $
Revision 1.2  95/01/29  13:27:10  NT_Test_Machine
*** empty log message ***

Revision 1.1  90/05/16  02:37:08  Anthony_Scian
.

 * Revision 3.0  90/05/15  22:37:05  kgorlen
 * Release for 1st edition.
 * 
*/

#define white_space "\040\011\012"

extern char* gettok(char* inbuf,char* tmnl);
#endif /* GETTOKH */
