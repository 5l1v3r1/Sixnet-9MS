#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <pcap.h>
#include "swm.h"

// CPU port register
#define CPUPORT 0x9
#define CPUREG (CPUPORT + 0x10)


pcap_t* swmpcap(){
	pcap_t *handle;
	char dev[] = "eth0";
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "not port 22";
	bpf_u_int32 mask;
	bpf_u_int32 net;
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1){
		printf("WARNING: Couldn't get netmask for device\n");
		net = 0;
		mask = 0;
	}
	handle = pcap_open_live(dev, 65535, 1, 1000, errbuf);
	if (handle == NULL){
		printf("ERROR: Couldn't open device!\n");
		exit(2);
	}
	if(pcap_compile(handle, &fp, filter_exp, 0, net) == -1){
		printf("ERROR: Couldn't parse filter\n");
		exit(3);
	}
	if(pcap_setfilter(handle, &fp) == -1){
		printf("ERROR: Couldn't install filter\n");
		exit(4);
	}
	return handle;
}

void print_mirror_status(void* mii_thing){
	int i = 0x10;
	uint pstat;
	int ingress;
	int pav;
	int myportpav;
	int j;
	for(i = 0; i < 0xa; i++){
		myportpav = ~(1 << i);
		// retrieve port register 11 (0xb)
		// remember port registers start at 0x10-0x19
		pstat = mdio_read(mii_thing, i+0x10, 0xb);
		ingress = pstat >> 15;
		pav = pstat & 0x3ff;
		pav &= myportpav; // get rid of my port's '1' bit
		// not look to see where this port is mirroring to
		if(pav != 0){
			printf("C port %d is set to mirror to", i);
			// walk through bits to find each one that is set
			for(j = 0; j < 10; j++){
				if(pav & (1 << j)){
					printf(" %d ", j);
				}
			}
			if (ingress) printf("in both directions\n");
			else printf("in egress-only\n");
		}
	}
}
// Note that on the ET-9MS configuration, 
// Sixnet only allows one target port; you're not supposed to do multiple
// independent mirrors like you can with other switches (even though
// such a feat is actually possible with the switch).
// so we will search all the ports and find which port they are mirroring to;
// if the switch is already set with multiple mirrors we will return -1.
// if the switch has no mirroring the result will be -2.
int get_current_mirror_target(void* mii_thing){
	int i, myportpav, ingress, pav, j;
	uint pstat;
	int portarget[10] = {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2};
	for(i = 0; i < 0xa; i++){
                myportpav = ~(1 << i);
                // retrieve port register 11 (0xb)
                // remember port registers start at 0x10-0x19
                pstat = mdio_read(mii_thing, i+0x10, 0xb);
                ingress = pstat >> 15;
                pav = pstat & 0x3ff;
                pav &= myportpav; // get rid of my port's '1' bit
                // not look to see where this port is mirroring to
                if(pav != 0){
                        // walk through bits to find each one that is set
                        for(j = 0; j < 10; j++){
                                if(pav & (1 << j)){
                                        portarget[i] = j;
                                }
                        }
                }
        }
	int targetport = -2;
	for(i = 0; i < 0xa; i++){
		if (portarget[i] != -2){
			if (targetport == -2){
				targetport = portarget[i];
				continue;
			}
			if (targetport != portarget[i]){
				return -1;
			}
		}
	}
	return targetport;
}

// redirect all mirror targets to the port defined by dest, with direction
// direction is 0 for egress only or 1 for both
// src and dest are the switchchip ports, not the case ports
int mirror_redirect(void* mii_thing, int src, int dest, int direction){
	// need to calculate the value for the register
	// we have to write this to the src port's mii table entry
	int regval = 1 << src;
	regval |= direction << 15;
	src += 0x10; // add 16 to get Cport
	regval |= 1 << dest;
	// write the regval out to source port's 0xb register
	mdio_write(mii_thing, src, 0xb, regval);
	printf("I would do mirror redirect via %08x to port %08x\n", regval, src);
	return 0;
}

int usage(name){
	printf("Usage: %s <read/write> <phy_id> <location> [value]\n");
	exit(1);
}

#define MODE_READ 1
#define MODE_WRITE 2

int main(int argc, char* argv[]){
	void* mii_thing = findSwm();
	int mode = 0;
	int phy;
	int loc;
	int val;
	int ret = 0;
	if (argc < 3) {
		usage(argv[0]);
	}
	if (strcmp(argv[1], "read") == 0){
		mode = MODE_READ;
	}
	else if (strcmp(argv[1], "write") == 0){
		mode = MODE_WRITE;
	}
	else{
		usage(argv[0]);
	}
	if (MODE_WRITE == mode && argc != 5) {
		usage(argv[0]);
	}
	if (MODE_READ == mode && argc != 4){
		usage(argv[0]);
	}
	phy = atoi(argv[2]);
	loc = atoi(argv[3]);
	if (MODE_WRITE == mode){
		val = atoi(argv[4]);
	}
	// assume atoi's work I guess
	if (MODE_READ == mode){
		printf("%02x/%02x: %04x\n", phy, loc, mdio_read(mii_thing, phy, loc));
	}
	if (MODE_WRITE == mode){
		ret = mdio_write(mii_thing, phy, loc, val);
	}
	return ret;
}
