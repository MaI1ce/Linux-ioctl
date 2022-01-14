#include <sys/ioctl.h>//input-putput control
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


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


    for (; it != end; ++it) {
        printf("interface name : %s\n", it->ifr_name);
        if (ioctl(sock, SIOCGIFINDEX, it) == 0) {
            printf("interface index: %d\n", it->ifr_ifindex;
        }
        else {
            temp_errno = errno;
            printf("ioctl error: %s \n",strerror(temp_errno));
            close(sock);
            return -1;
        }     
    }

    close(sock);
    return 0;
}
 //if (ioctl(socket, SIOCGIFFLAGS, &ifr) == 0) {
//if (! (ifr->ifr_flags & IFF_LOOPBACK)) { // don't count loopback
