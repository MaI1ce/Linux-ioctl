/*
    Program to study basics of ioctl interface
    last change: 16.01.2022
    autor      : Vladyslav Shapoval
*/

#include <sys/ioctl.h>//input-output control
#include <sys/socket.h>//struct sockaddr
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>//struct sockaddr_in 
#include <string.h>//strcpy
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/if_arp.h>//ioctl's for ARP
#include <arpa/inet.h> //char *inet_ntoa(struct in_addr in);

/*struct to save info from driver*/
struct interface {
    char   name[256];
    int    index;
    struct in_addr addr;
    struct in_addr mask;
    short  flags;
    short  mac_family;
    char   mac[8];
} inf;

void show_info (struct interface * info);

int main (void) 
{

    char buf[1024];

    struct ifconf ifc;
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    
    int temp_errno = 0;
    
    /*open socket*/
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1){ /*handle error*/
        temp_errno = errno;
        printf("socket error: %s \n",strerror(temp_errno));
        close(sock);
        return -1;
    }

    /* get list of interfaces*/
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { 
        /* handle error */ 
        temp_errno = errno;
        printf("ioctl error: %s \n",strerror(temp_errno));
        close(sock);
        return -1;
    }

    /*find end of the list*/
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    /*list of requests to driver*/
    unsigned long int  request[5] = {SIOCGIFINDEX, SIOCGIFFLAGS, SIOCGIFADDR,
                                     SIOCGIFNETMASK, SIOCGIFHWADDR};


    struct sockaddr_in* src = NULL;
    struct interface req;
    int i = 0;
    /*for each interface in the list*/
    for (; it != end; ++it) {
        strcpy(req.name, it->ifr_name);
        for(i = 0; i < 5; ++i){
            /*request to driver*/
            if (ioctl(sock, request[i], it )== -1) {
                temp_errno = errno;
                printf("ioctl error: %s \n",strerror(temp_errno));
                close(sock);
                return -1;
            }
            switch(i){
                case 0:
                    req.index = it->ifr_ifindex;
                    break;
                case 1:
                    req.flags = it->ifr_flags;
                    break;
                case 2:
                    src = (struct sockaddr_in*)&(it->ifr_addr);
                    req.addr.s_addr = src->sin_addr.s_addr;
                    break;
                case 3:
                    src = (struct sockaddr_in*)&(it->ifr_netmask);
                    req.mask.s_addr = src->sin_addr.s_addr;
                    break;
                case 4:
                    req.mac_family = it->ifr_hwaddr.sa_family;
                    memcpy(req.mac, it->ifr_hwaddr.sa_data, 6);
            }
        }
        show_info(&req);
    }

    close(sock);
    return 0;
}


void show_info (struct interface * info)
{
    const unsigned char* mac=(unsigned char*)info->mac;
    
    printf("interface name  : %s\n", info->name);
    
    printf("\tindex   : %d\n",info->index);

    printf("\taddress : %s\n",inet_ntoa(info->addr));

    printf("\tnetmask : %s\n",inet_ntoa(info->mask));

    if (info->flags & IFF_LOOPBACK){
        printf("\tloopback\n");
    }
    else if (info->mac_family == ARPHRD_ETHER) {
        printf("\tmac addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
    else {
        printf("\tnot an Ethernet interface\n");
    }
}
/*
#include <netinet/in.h>

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};


#include <sys/socket>

struct sockaddr{
    sa_family_t   sa_family       address family
    char          sa_data[]       socket address (variable-length data)
};
*/
