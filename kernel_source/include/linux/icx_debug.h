/*
 *	Copyright 2016 Sony Corporation
 *	File changed on 2016-01-22
 */
/*
 *	icx_debug.h -- sony icx debug
 *
 *	Copyright 2016 Sony Corporation
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  version 2 of the  License.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#ifndef __ICX_DEBUG_H__
#define __ICX_DEBUG_H__

#include <linux/ktime.h>

/**********************/
/* statistics_command */
/*********************/

typedef struct{
	unsigned int p0 :  4;
	unsigned int p1 : 28;
	unsigned int p2 : 12;
	unsigned int p3 : 20;
}STATISTICS_DATA;

int icx_debug_statistics_alloc(int data_count);
int icx_debug_statistics_free(void);
int icx_debug_statistics_clear(void);
int icx_debug_statistics_start(void);
int icx_debug_statistics_stop(void);
int icx_debug_statistics_mode(int mode);
int icx_debug_statistics_stat(void);
int icx_debug_statistics_debug(void);
int icx_debug_statistics_save(
	unsigned int p0, /*  4 bits */
	unsigned int p1, /* 28 bits */
	unsigned int p2, /* 12 bits */
	unsigned int p3  /* 20 bits */
);

/*********/
/* timer */
/*********/

int icx_debug_get_usec(ktime_t starttime);

#endif /* __ICX_DEBUG_H__ */
