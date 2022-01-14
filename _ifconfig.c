#include <sys/ioctl.h>//input-output control
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

struct interface {
    char   name[256];
    int    index;
    struct in_addr addr;
    struct in_addr mask;
    short  flags;
    short  mac_family;
    char   mac[8];
} inf;

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

int main (void) 
{

    char buf[1024];

    struct ifreq ifr;
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

    unsigned long int  request[5] = {SIOCGIFINDEX, SIOCGIFFLAGS, SIOCGIFADDR,
                                     SIOCGIFNETMASK, SIOCGIFHWADDR};

    struct sockaddr_in* src = NULL;
    struct sockaddr_in* dst = NULL;
    struct ifreq temp_req;
    struct interface req;
    int i = 0;
    for (; it != end; ++it) {
        strcpy(temp_req.ifr_name, it->ifr_name);
        strcpy(req.name, it->ifr_name);
        for(i = 0; i < 5; ++i){
            if (ioctl(sock, request[i], &temp_req )== -1) {
                temp_errno = errno;
                printf("ioctl error: %s \n",strerror(temp_errno));
                close(sock);
                return -1;
            }
            switch(i){
                case 0:
                    req.index = temp_req.ifr_ifindex;
                    break;
                case 1:
                    req.flags = temp_req.ifr_flags;
                    break;
                case 2:
                    src = (struct sockaddr_in*)&(temp_req.ifr_addr);
                    req.addr.s_addr = src->sin_addr.s_addr;
                    break;
                case 3:
                    src = (struct sockaddr_in*)&(temp_req.ifr_netmask);
                    req.mask.s_addr = src->sin_addr.s_addr;
                    break;
                case 4:
                    req.mac_family = temp_req.ifr_hwaddr.sa_family;
                    memcpy(req.mac, temp_req.ifr_hwaddr.sa_data, 6);
                    break;
            }
        }
        show_info(&req);
    }

    close(sock);
    return 0;
}
