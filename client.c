

 /*======================================================
  //2015-4-29 hxl trimed the original client to a simpler project,
  // which contains only

  =======================================================*/
/*
 *  ======== client.c ========
 *
 * TCP/IP Network Client example ported to use BIOS6 OS.
 *
 * Copyright (C) 2007, 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <stdio.h>
#include <xdc/runtime/Error.h> //hxl added
#include <xdc/runtime/System.h>

#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
//#include <ti/ndk/inc/tools/console.h> //hxl: delete
#include <ti/ndk/inc/tools/servers.h> //hxl:完整目录：C:\ti\ndk_2_21_01_38\packages\ti\ndk\inc\tools
/* ---BIOS6 include ------------*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h> //hxl added
#include <ti/sysbios/knl/Semaphore.h>// hxl
#include <ti/sysbios/knl/Swi.h> //hxl added 2015-9-21
/* Platform utilities include */
#include <ti/platform/platform.h>

/* Resource manager for QMSS, PA, CPPI */
#include "ti/platform/resource_mgr.h"

//-------hxl added the flollowing head files---------------
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/hal/Hwi.h>
#include "SRIO_loopbackDioIsr.h"//hxl added
#include "task_TCP_Svr.h"
//---------------------------------------------------------------------------

// Title String
//
char *VerStr = "\nTCP/IP Stack Client\n";//hxl:modified

// Our NETCTRL callback functions
static void   NetworkOpen();
static void   NetworkClose();
static void   NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd );

// Fun reporting function
static void   ServiceReport( uint Item, uint Status, uint Report, HANDLE hCfgEntry );  //hxl modifed

/* Platform Information - we will read it form the Platform Library */
//platform_info	gPlatformInfo;
/*====================================================================
 * hxl:
 * 2015-7-21 全局变量的定义
 * 2015-9-21 hwi，
===================================================================== */

//extern Swi_Handle swi_handle0 ;//hxl added
Task_Handle hTCPtask1;
Task_Handle hTCPtask2;

//--------------ended hxl:全局变量的定义-----------------------------------------------

//---------------hxl added-----------------------------------------------------------
// External references
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Configuration
//

char *HostName    = "tidsp";
char *LocalIPAddr = "192.168.2.100";
//char *LocalIPMask = "255.255.255.0";    // Not used when using DHCP
char *LocalIPMask = "255.255.254.0";    // Not used when using DHCP  hxl modified
char *GatewayIP   = "192.168.2.101";    // Not used when using DHCP
char *DomainName  = "demo.net";         // Not used when using DHCP
char *DNSServer   = "0.0.0.0";          // Used when set to anything but zero


// Simulator EMAC Switch does not handle ALE_LEARN mode, so please configure the
// MAC address of the PC where you want to launch the webpages and initiate PING to NDK */

Uint8 clientMACAddress [6] = {0x5C, 0x26, 0x0A, 0x69, 0x44, 0x0B}; /* MAC address for my PC */

UINT8 DHCP_OPTIONS[] = { DHCPOPT_SERVER_IDENTIFIER, DHCPOPT_ROUTER };


/*************************************************************************
 *  @b EVM_init()
 *
 *  @n
 *
 *  Initializes the platform hardware. This routine is configured to start in
 * 	the evm.cfg configuration file. It is the first routine that BIOS
 * 	calls and is executed before Main is called. If you are debugging within
 *  CCS the default option in your target configuration file may be to execute
 *  all code up until Main as the image loads. To debug this you should disable
 *  that option.
 *
 *  @param[in]  None
 *
 *  @retval
 *      None
 ************************************************************************/
void EVM_init()
{
	platform_init_flags  	sFlags;
	platform_init_config 	sConfig;
	/* Status of the call to initialize the platform */
	int32_t pform_status;
    int i=0;// hxl for test
	/*
	 * You can choose what to initialize on the platform by setting the following
	 * flags. Things like the DDR, PLL, etc should have been set by the boot loader.
	*/
	memset( (void *) &sFlags,  0, sizeof(platform_init_flags));
	memset( (void *) &sConfig, 0, sizeof(platform_init_config));

	sFlags.pll  = 1;//0;	/* PLLs for clocking  	*/ //hxl modified it for PLL config to 1000MHz(同时sConfig.pllm = 0)
	sFlags.ddr  = 0;//0;   	/* External memory 		*/
    sFlags.tcsl = 1;	/* Time stamp counter 	*/ //2016-6-29 hxl:库文件中默认是开启
#ifdef _SCBP6618X_
    sFlags.phy  = 0;	/* Ethernet 			*/
#else
    sFlags.phy  = 1;	/* Ethernet 			*/
#endif
  	sFlags.ecc  = 0;	/* Memory ECC 			*/

    sConfig.pllm = 0;	/* Use libraries default clock divisor */

	pform_status = platform_init(&sFlags, &sConfig);
	//-----------used for test hxl--------------------------
	while (i<5) {
		(void) platform_led(2, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
		(void) platform_delay(10000);
		(void) platform_led(2, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
		(void) platform_delay(10000);
		i++; //added for test
	}
	//-------------end used for test hxl------------------------

	/* If we initialized the platform okay */
	if (pform_status != Platform_EOK) {
		/* Initialization of the platform failed... die */
		while (1) {
			(void) platform_led(1, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
			(void) platform_delay(50000);
			(void) platform_led(1, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
			(void) platform_delay(50000);
		}
	}
}

//
// Main Thread
//
//
int StackTest()
{
    int               rc,i;
    HANDLE            hCfg;
  //  CI_SERVICE_TELNET telnet; //hxl modified
  //  CI_SERVICE_HTTP   http;   //hxl modified
	// Clear the state of the User LEDs to OFF to indacate entering  StackTest() //
	for (i=0; i < gPlatformInfo.led[PLATFORM_USER_LED_CLASS].count; i++)//2016-6-30 hxl:added this from stackTest()
	{
	 (void) platform_led(i, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
	}
    //
    // THIS MUST BE THE ABSOLUTE FIRST THING DONE IN AN APPLICATION before
    //  using the stack!!
    //
    //rc = NC_SystemOpen( NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT );//
	rc = NC_SystemOpen( OS_SCHEDULER_HIGHPRI, NC_OPMODE_INTERRUPT );///hxl:modified to High,优先级8
    if( rc )
    {
        platform_write("NC_SystemOpen Failed (%d)\n",rc);
        for(;;);
    }

    // Print out our banner
    platform_write(VerStr);

    //
    // Create and build the system configuration from scratch.
    //

    // Create a new configuration
    hCfg = CfgNew();
    if( !hCfg )
    {
        platform_write("Unable to create configuration\n");
        goto main_exit;
    }

    // We better validate the length of the supplied names
    if( strlen( DomainName ) >= CFG_DOMAIN_MAX ||
        strlen( HostName ) >= CFG_HOSTNAME_MAX )
    {
        platform_write("Names too long\n");
        goto main_exit;
    }

    // Add our global hostname to hCfg (to be claimed in all connected domains)
    CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0,
                 strlen(HostName), (UINT8 *)HostName, 0 );

    // If the IP address is specified, manually configure IP and Gateway
#if defined(_SCBP6618X_) || defined(_EVMTCI6614_) || defined(DEVICE_K2H) || defined(DEVICE_K2K)
    /* SCBP6618x, EVMTCI6614, EVMK2H, EVMK2K always uses DHCP */
    if (0)
#else
    if (!platform_get_switch_state(1))
#endif
    {
        CI_IPNET NA;
        CI_ROUTE RT;
        IPN      IPTmp;

        // Setup manual IP address
        bzero( &NA, sizeof(NA) );
        NA.IPAddr  = inet_addr(LocalIPAddr);
        NA.IPMask  = inet_addr(LocalIPMask);
        strcpy( NA.Domain, DomainName );
        NA.NetType = 0;

        // Add the address to interface 1
        CfgAddEntry( hCfg, CFGTAG_IPNET, 1, 0,
                           sizeof(CI_IPNET), (UINT8 *)&NA, 0 );

        // Add the default gateway. Since it is the default, the
        // destination address and mask are both zero (we go ahead
        // and show the assignment for clarity).
        bzero( &RT, sizeof(RT) );
        RT.IPDestAddr = 0;
        RT.IPDestMask = 0;
        RT.IPGateAddr = inet_addr(GatewayIP);//hxl:inet_addr将字符串转换为ip地址

        // Add the route
        CfgAddEntry( hCfg, CFGTAG_ROUTE, 0, 0,
                           sizeof(CI_ROUTE), (UINT8 *)&RT, 0 );

        // Manually add the DNS server when specified
        IPTmp = inet_addr(DNSServer);
        if( IPTmp )
            CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                         0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
    }

   // Else we specify DHCP
    else
    {
        CI_SERVICE_DHCPC dhcpc;

        platform_write("Configuring DHCP client\n");

        // Specify DHCP Service on IF-1
        bzero( &dhcpc, sizeof(dhcpc) );
        dhcpc.cisargs.Mode   = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx  = 1;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        dhcpc.param.pOptions = DHCP_OPTIONS;
        dhcpc.param.len = 2;

        CfgAddEntry( hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                     sizeof(dhcpc), (UINT8 *)&dhcpc, 0 );
    }

    //
    // Configure IPStack/OS Options
    //

    // We don't want to see debug messages less than WARNINGS
    //rc = DBG_INFO;
    rc=DBG_NONE; //2016-8-2 hxl 禁止打印ndk调试信息
    CfgAddEntry( hCfg, CFGTAG_OS, CFGITEM_OS_DBGPRINTLEVEL,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    //
    // This code sets up the TCP and UDP buffer sizes
    // (Note 8192 is actually the default. This code is here to
    // illustrate how the buffer and limit sizes are configured.)
    //

    // TCP Transmit buffer size
    rc = 81920;//65536;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPTXBUF,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    // TCP Receive buffer size (copy mode)
    //rc = 8192;
    rc=81920;//2017-10-23 hxl:modified
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXBUF,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    // TCP Receive limit (non-copy mode)
    //rc = 8192;
    rc = 16384;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXLIMIT,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    // UDP Receive limit
    rc = 8192;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKUDPRXLIMIT,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

#if 0
    // TCP Keep Idle (10 seconds)
    rc = 100;
    //rc = 1000;
    //   This is the time a connection is idle before TCP will probe
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPIDLE,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    // TCP Keep Interval (1 second)
    //   This is the time between TCP KEEP probes
    rc = 10;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPINTVL,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    // TCP Max Keep Idle (5 seconds)
    //   This is the TCP KEEP will probe before dropping the connection
    rc = 50;//
    //rc = 500;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPMAXIDLE,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );
#endif

    //
    // Boot the system using this configuration
    //
    // We keep booting until the function returns 0. This allows
    // us to have a "reboot" command.
    // hxl:NC_NetStart 调用stack event scheduler
    do
    {
        rc = NC_NetStart( hCfg, NetworkOpen, NetworkClose, NetworkIPAddr );//hxl:使用函数指针传递NetworkOpen()等。
    } while( rc > 0 );//hxl:NetworkClose函数调用函数NC_NetStop

    // Free the WEB files
   // RemoveWebFiles(); //hxl modified

    // Delete Configuration
    CfgFree( hCfg );

    // Close the OS
main_exit:
    NC_SystemClose();//
    return(0);
}






//
// NetworkOpen
//
// This function is called after the configuration has booted


static void NetworkOpen()   //hxl: StackTest() calls NetworkOPen()
{
	// Create our local servers
		UINT32 Arg1,Arg2,Arg3;//任务函数的参数
		Arg1=0;
		Arg2=0;
		Arg3=0;
	    char *name1="TCPtask1";
	    //char *name2="TCPtask2";
		Error_Block eb;
		Error_init(&eb);
		/* Create 1 task with priority 5,stackSize = 2048 */
		hTCPtask1 =TaskCreate(task_TCP_Svr,name1,6,4096,Arg1,Arg2,Arg3);//hxl:任务函数的创建;
		if (hTCPtask1 == NULL) {
		System_abort("TCP Task create failed");

		}
		/*hTCPtask2 =TaskCreate(task_TCP_sendSvr,name2,5,2048,Arg1,Arg2,Arg3);//hxl:任务函数的创建;
		if (hTCPtask2 == NULL) {
		System_abort("TCP Task create failed");
		}*/


}

//
// NetworkClose
//
// This function is called when the network is shutting down,
// or when it no longer has any IP addresses assigned to it.
//
static void NetworkClose() //    hxl modifeid
{
    TaskDestroy(hTCPtask1);
    //TaskDestroy(hTCPtask2);


}


//
// NetworkIPAddr
//
// This function is called whenever an IP address binding is
// added or removed from the system.
//
static void NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd )
{
    static uint fAddGroups = 0;
    IPN IPTmp;

    if( fAdd )
        platform_write("Network Added: ");
    else
        platform_write("Network Removed: ");

    // Print a message
    IPTmp = ntohl( IPAddr );
    platform_write("If-%d:%d.%d.%d.%d\n", IfIdx,
            (UINT8)(IPTmp>>24)&0xFF, (UINT8)(IPTmp>>16)&0xFF,
            (UINT8)(IPTmp>>8)&0xFF, (UINT8)IPTmp&0xFF );

    // This is a good time to join any multicast group we require
    if( fAdd && !fAddGroups )
    {
        fAddGroups = 1;
//              IGMPJoinHostGroup( inet_addr("224.1.2.3"), IfIdx );
    }

}

//
// DHCP_reset()
//
// Code to reset DHCP client by removing it from the active config,
// and then reinstalling it.
//
// Called with:
// IfIdx    set to the interface (1-n) that is using DHCP.
// fOwnTask set when called on a new task thread (via TaskCreate()).
//
void DHCP_reset( uint IfIdx, uint fOwnTask )
{
    CI_SERVICE_DHCPC dhcpc;
    HANDLE h;
    int    rc,tmp;
    uint   idx;

    // If we were called from a newly created task thread, allow
    // the entity that created us to complete
    if( fOwnTask )
        TaskSleep(500);

    // Find DHCP on the supplied interface
    for(idx=1; ; idx++)
    {
        // Find a DHCP entry
        rc = CfgGetEntry( 0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT,
                          idx, &h );
        if( rc != 1 )
            goto RESET_EXIT;

        // Get DHCP entry data
        tmp = sizeof(dhcpc);
        rc = CfgEntryGetData( h, &tmp, (UINT8 *)&dhcpc );

        // If not the right entry, continue
        if( (rc<=0) || dhcpc.cisargs.IfIdx != IfIdx )
        {
            CfgEntryDeRef(h);
            h = 0;
            continue;
        }

        // This is the entry we want!

        // Remove the current DHCP service
        CfgRemoveEntry( 0, h );

        // Specify DHCP Service on specified IF
        bzero( &dhcpc, sizeof(dhcpc) );
        dhcpc.cisargs.Mode   = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx  = IfIdx;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        CfgAddEntry( 0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                     sizeof(dhcpc), (UINT8 *)&dhcpc, 0 );
        break;
    }

RESET_EXIT:
    // If we are a function, return, otherwise, call TaskExit()
    if( fOwnTask )
        TaskExit();
}


static char *TaskName[]  = { "Telnet","HTTP","NAT","DHCPS","DHCPC","DNS" };
static char *ReportStr[] = { "","Running","Updated","Complete","Fault" };
static char *StatusStr[] = { "Disabled","Waiting","IPTerm","Failed","Enabled" };
static void ServiceReport( uint Item, uint Status, uint Report, HANDLE h )
{
    platform_write( "Service Status: %-9s: %-9s: %-9s: %03d\n",
            TaskName[Item-1], StatusStr[Status],
            ReportStr[Report/256], Report&0xFF );

    //
    // Example of adding to the DHCP configuration space
    //
    // When using the DHCP client, the client has full control over access
    // to the first 256 entries in the CFGTAG_SYSINFO space.
    //
    // Note that the DHCP client will erase all CFGTAG_SYSINFO tags except
    // CFGITEM_DHCP_HOSTNAME. If the application needs to keep manual
    // entries in the DHCP tag range, then the code to maintain them should
    // be placed here.
    //
    // Here, we want to manually add a DNS server to the configuration, but
    // we can only do it once DHCP has finished its programming.
    //
    if( Item == CFGITEM_SERVICE_DHCPCLIENT &&
        Status == CIS_SRV_STATUS_ENABLED &&
        (Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPADD) ||
         Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPRENEW)) )
    {
        IPN IPTmp;

        // Manually add the DNS server when specified
        IPTmp = inet_addr(DNSServer);
        if( IPTmp )
            CfgAddEntry( 0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                         0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
#if 0
        // We can now check on what the DHCP server supplied in
        // response to our DHCP option tags.
        CheckDHCPOptions();
#endif

    }

    // Reset DHCP client service on failure
    if( Item==CFGITEM_SERVICE_DHCPCLIENT && (Report&~0xFF)==NETTOOLS_STAT_FAULT )
    {
        CI_SERVICE_DHCPC dhcpc;
        int tmp;

        // Get DHCP entry data (for index to pass to DHCP_reset).
        tmp = sizeof(dhcpc);
        CfgEntryGetData( h, &tmp, (UINT8 *)&dhcpc );

        // Create the task to reset DHCP on its designated IF
        // We must use TaskCreate instead of just calling the function as
        // we are in a callback function.
        TaskCreate( DHCP_reset, "DHCPreset", OS_TASKPRINORM, 0x1000,
                    dhcpc.cisargs.IfIdx, 1, 0 );
    }
}

/*========软件中断SWI========*/

void beforeProcessSwi(UArg arg0, UArg arg1) //模拟信号处理过程

{

}
