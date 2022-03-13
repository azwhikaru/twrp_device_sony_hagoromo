/* 2013-07-16: File added and changed by Sony Corporation */
/*
 *  icx_usbconn.h
 *
 *  Copyright 2011,2014 Sony Corporation
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

#ifndef ICX_USBCONN_H
#define ICX_USBCONN_H

#include <asm/ioctl.h>

/* Command Code */
#define MSC_CONN_IOC_MAGIC      'g'
    #define MSC_GET_CSTAT       _IOR(MSC_CONN_IOC_MAGIC, 1, int)
    #define MSC_ALLOW_ACC       _IOW(MSC_CONN_IOC_MAGIC, 2, int)
    #define MSC_PROHIBIT_ACC    _IOW(MSC_CONN_IOC_MAGIC, 3, int)
    #define MSC_SET_LUN         _IOW(MSC_CONN_IOC_MAGIC, 4, unsigned int)    /* unused */
    #define MSC_GET_LUN         _IOR(MSC_CONN_IOC_MAGIC, 5, unsigned int)    /* unused */
    #define MSC_CONN_EXT        _IO (MSC_CONN_IOC_MAGIC, 6)                  /* unused */
#define MSC_CONN_IOC_MAXNR      6

/* USB Connect Status */
#define USB_DISCONNECT          0
#define USB_CONNECT             1
#define USB_FW_UPDATE           2
#define USB_MTP_CONNECT         3           /* unused */
#define USB_START_CONNECT       4           /* unused */


enum USB_CONN_ACC_STS {
    USB_CONN_ACC_INIT           = 0,
    USB_CONN_ACC_ALLOW,
    USB_CONN_ACC_PROHIBIT
};


struct conn_to_msc_func {
    int     (*change_status)(int sts);
};

struct msc_to_conn_func {
    void    (*allow_acc)(int);
};

struct conn_to_gadget_func {
    int     (*change_status)(int sts);
};

extern void usbconn_bind_msc(struct msc_to_conn_func *);

#endif  /* ICX_USBCONN_H */
