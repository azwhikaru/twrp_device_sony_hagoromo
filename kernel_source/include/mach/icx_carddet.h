/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  icx_carddet.h
 *
 *  Copyright 2012,2014 Sony Corporation
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

#ifndef ICX_CARDDET_H
#define ICX_CARDDET_H

#include <asm/ioctl.h>

/************/
/* User I/F */
/************/

/*
 * Command code 
 */
#define CARD_DET_IOC_MAGIC         'c'
#define CARD_DET_GET_INFO          _IOR(CARD_DET_IOC_MAGIC, 1, icx_card_info_t)
#define CARD_DET_CONTROL_POWER     _IOW(CARD_DET_IOC_MAGIC, 2, int)
#define CARD_DET_GET_XDET          _IOR(CARD_DET_IOC_MAGIC, 3, int)
#define CARD_DET_REQ_DETECT        _IO (CARD_DET_IOC_MAGIC, 4)
#define CARD_DET_IOC_MAXNR         4

/*
 * Card status
 */

/* detection status */
#define ICX_CARDDET_STATE_NG        -1
#define ICX_CARDDET_STATE_REMOVED   0
#define ICX_CARDDET_STATE_DETECTED  1
#define ICX_CARDDET_STATE_SUSPEND   2
#define ICX_CARDDET_STATE_INSERTED  3

/* card type */
#define ICX_CARDDET_TYPE_NONE       -1
#define ICX_CARDDET_TYPE_MS         0
#define ICX_CARDDET_TYPE_SD         1

/* card information */
typedef struct icx_card_info {
    int state;               /* detected/removed */
    int type;                /* SD/MS/none       */
    int ro;                  /* read only        */
} icx_card_info_t;


/**************/
/* Driver I/F */
/**************/

/* result code */
#define ICX_CARDDET_RESULT_NG    0
#define ICX_CARDDET_RESULT_OK    1

#if defined(CONFIG_ICD_0425) || defined(CONFIG_WM_FY14)
/* gpio state */
#define ICX_CARDDET_GPIO_DETECT        0
#define ICX_CARDDET_GPIO_REMOVE        1

/* irq type */
#define ICX_CARDDET_IRQ_TYPE_DETECT    IRQ_TYPE_EDGE_FALLING
#define ICX_CARDDET_IRQ_TYPE_REMOVE    IRQ_TYPE_EDGE_RISING

#else
/* gpio state */
#define ICX_CARDDET_GPIO_DETECT        1
#define ICX_CARDDET_GPIO_REMOVE        0

/* irq type */
#define ICX_CARDDET_IRQ_TYPE_DETECT    IRQ_TYPE_EDGE_RISING
#define ICX_CARDDET_IRQ_TYPE_REMOVE    IRQ_TYPE_EDGE_FALLING
#endif

struct icx_carddet_device {
    void *host;
};


struct carddet_eint_ops {
    int(*notifier_call)(int irq, void *arg);
};

struct carddet_func_tbl {
    void(*card_removed)(void);
};

struct carddet_to_msc_func {
	int (*get_status)(void);
};

#endif  /* ICX_CARDDET_H */


