#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int lanGetIpAddress(const char *ifName, char *ipAddr, int addrLen)
{
    int ret = -1;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    int sock;

    if(ifName == NULL || ipAddr == NULL)
    {
        return -1;
    }

    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
    //printf("ifname:%s\n", ifr.ifr_name);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > (ret = ioctl(sock, SIOCGIFADDR, &ifr)))
    {
        perror("ioctl get SIOCGIFADDR failed :");
        //printf("ioctl get SIOCGIFADDR failed!!");
        return -1;
    }
    close(sock);

    sin = (struct sockaddr_in *)&(ifr.ifr_addr);
    if (NULL == inet_ntoa(sin->sin_addr))
    {
        return -1;
    }
    strncpy(ipAddr, inet_ntoa(sin->sin_addr), addrLen);

    return ret;
}

// 通过网口号获取mac
int lanGetMACaddress(const char *eth_inf, char *mac, int macLen)
{
    struct ifreq ifr;
    int sd;

    bzero(&ifr, sizeof(struct ifreq));
    if( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        //perror("get mac address socket creat error\n");
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        //printf("get %s mac address error\n", eth_inf);
        close(sd);
        return -1;
    }

    snprintf(mac, macLen, "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    close(sd);

    return 0;
}

// 将mac字符串转为数值
int lanGetMacValue(const char *mac_addr, unsigned char *mac)
{
    if(mac_addr == NULL || mac == NULL)
    {
        return -1;
    }
    sscanf(mac_addr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
        &mac[0],
        &mac[1],
        &mac[2],
        &mac[3],
        &mac[4],
        &mac[5]);
    return 0;
}

#if 0
void demo__lanGetIpAddress(void)
{
    char ip[64];
    char mac_string[64];
    unsigned char mac_bytes[6];
    const char *if = "eth0"
    int ret;
    int i;
    // 通过网口获取IP
    ret = lanGetIpAddress(if, ip, sizeof(ip));
    if(ret == 0)
    {
        printf("IP is %s\n", ip);
    } else
    {
        printf("get IP failed, ret = [%d] \n", ret);
    }
    // 通过网口获取MAC(字符串)
    ret = lanGetMacAddress(if, mac_string, sizeof(mac_string));
    if(ret == 0)
    {
        printf("mac is %s\n", mac_string);
    } else
    {
        printf("get mac failed, ret = [%d] \n", ret);
    }
    // 将mac字符串转为数值
    ret = lanGetMacValue(mac_string, mac_bytes, sizeof(mac_bytes));
#if 0
    for(i = 0; i < 6; i++)
    {
        printf("%02x \n", mac_bytes[i]);
    }
#endif
}
#endif
