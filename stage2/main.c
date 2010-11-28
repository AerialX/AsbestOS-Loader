/*  main.c - AsbestOS Stage2 main C entrypoint

Copyright (C) 2010  Hector Martin "marcan" <hector@marcansoft.com
Copyright (C) 2010  Francisco Muñoz "hermes" <www.elotrolado.net>

This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "types.h"
#include "lv1call.h"


enum {
	DEV_TYPE_STOR_DISK = 0,
	DEV_TYPE_ETH = 3,
	DEV_TYPE_USB = 4,
	DEV_TYPE_STOR_ROM = 5,
	DEV_TYPE_SB_GPIO = 6,
	DEV_TYPE_STOR_FLASH = 14,
};

#define _PS(s) (s "\0\0\0\0\0\0\0\0")
#define S2I(s) ( \
	(((u64)_PS(s)[0])<<56) | \
	(((u64)_PS(s)[1])<<48) | \
	(((u64)_PS(s)[2])<<40) | \
	(((u64)_PS(s)[3])<<32) | \
	(((u64)_PS(s)[4])<<24) | \
	(((u64)_PS(s)[5])<<16) | \
	(((u64)_PS(s)[6])<<8) | \
	(((u64)_PS(s)[7])<<0))

#define PS3_LPAR_ID_PME 1

#define FIELD_FIRST(s, i) ((S2I(s)>>32) + (i))
#define FIELD(s, i) (S2I(s) + (i))

int close_all_devs(void)
{
	u64 v2;
	u64 bus_ndx;
	s64 result;
	int gelic_bus=-1, gelic_dev=-1;


	//if (find_device_by_type(DEV_TYPE_ETH, 0, &gelic_bus, &gelic_dev, NULL) != 0) {
//		gelic_bus = gelic_dev = -1;
//	}

	int closed = 0;

	//printf("Closing all devices...\n");
	for (bus_ndx=0; bus_ndx<10; bus_ndx++) {
		u64 bus_id=0, bus_type=0, num_dev=0;

		result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
											   FIELD("type",0), 0, 0, &bus_type, &v2);
		if (result)
			continue;
		result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
											   FIELD("id",0), 0, 0, &bus_id, &v2);
		if (result)
			continue;
		result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
											   FIELD("num_dev",0), 0, 0, &num_dev, &v2);
		if (result)
			continue;

		//printf("Bus #%ld id %ld type %ld num_dev %ld\n", bus_ndx, bus_id, bus_type, num_dev);
		u64 dev_ndx;
		for (dev_ndx=0; dev_ndx<num_dev; dev_ndx++) {
			s64 dev_id=0, dev_type=0, dev_intr=0;

			result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
												   FIELD("dev",dev_ndx), FIELD("id",0), 0, (u64*)&dev_id, &v2);
			if (result)
				dev_id = -1;
			result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
												   FIELD("dev",dev_ndx), FIELD("type",0), 0, (u64*)&dev_type, &v2);
			if (result)
				dev_type = -1;
			result = lv1_get_repository_node_value(PS3_LPAR_ID_PME, FIELD_FIRST("bus",bus_ndx),
												   FIELD("dev",dev_ndx), FIELD("intr",0), 0, (u64*)&dev_intr, &v2);
			if (result)
				dev_intr = -1;

			//printf("- Dev #%ld id %ld type %ld intr %ld ", dev_ndx, dev_id, dev_type, dev_intr);
			if (bus_id == gelic_bus && dev_id == gelic_dev)
				;//printf("KEPT (gelic)\n");
			else if (lv1_close_device(bus_id, dev_id) == 0) {
				//printf("CLOSED\n");
				closed++;
			} else {
				//printf("\n");
			}
		}
	}
	return closed;
}

void lv2_cleanup(void)
{
	s64 result;
	int i,j;
	//printf("Cleaning up after Lv-2...\n");

	

	if (lv1_gpu_close() == 0)
		;//printf("GPU closed\n");

	if (lv1_deconfigure_virtual_uart_irq() == 0)
		;//printf("Deconfigured VUART IRQ\n");

	u64 ppe_id;
	result = lv1_get_logical_ppe_id(&ppe_id);

	//printf("PPE id: %ld\n", ppe_id);

#if 1
	u64 vas_id;
	result = lv1_get_virtual_address_space_id_of_ppe(ppe_id , &vas_id );
	//printf("VAS id: %ld\n", vas_id);

	result = lv1_select_virtual_address_space(0);
	if (result == 0)
		;//printf("Selected VAS 0\n");
#endif


	int count = 0;

	for (i=0; i<512; i++)
		for (j=0; j<2; j++)
			if (lv1_disconnect_irq_plug_ext(ppe_id, j, i) == 0)
				count++;

	//printf("Disconnected %d IRQ plugs\n", count);

	for (i=0, count=0; i<256; i++)
		if (lv1_destruct_io_irq_outlet(i) == 0)
			count++;
	//printf("Destructed %d IRQ outlets\n", count);

	count = 0;
	if (lv1_configure_irq_state_bitmap(ppe_id, 0, 0) == 0) count++;
	if (lv1_configure_irq_state_bitmap(ppe_id, 1, 0) == 0) count++;
	//printf("Removed %d state bitmaps\n", count);

//	printf("Closed %d devices\n", );

	close_all_devs();
	
	for (i=0; i<256; i++) {
		if (lv1_disable_logical_spe(i, 0) == 0)
			;//printf("Disabled logical spe %d\n", i);
		if (lv1_destruct_logical_spe(i) == 0)
			;//printf("Destroyed logical spe %d\n", i);
	}

	u64 size = 256*1024*1024;
	int blocks = 0;
	u64 total = 0;

	//printf("Allocating all possible lv1 memory...\n");

	u64 ctr_max = 0;

	while (size >= 4096) {
		u64 newaddr;
		u64 muid;
		u64 cur_ctr;
		result = lv1_allocate_memory(size, 12, 0, 0, &newaddr, &muid);
		if (result) {
			size >>= 1;
			continue;
		}
		cur_ctr = (newaddr & 0x03fffffff000)>>12;
		if (cur_ctr > ctr_max)
			ctr_max = cur_ctr;
		blocks++;
		total += size;
	}

	//printf("Allocated %ld bytes in %d blocks\n", total, blocks);
	//printf("Current allocation address counter is 0x%lx\n", ctr_max);

	//printf("Brute forcing Lv1 allocation regions...\n");

	u64 counter;
	u64 sbits;
	total = 0;
	blocks = 0;
	u64 prev = -1;

	u64 ctr_limit = 0x40000;

	for (counter = 0; counter <= ctr_limit; counter++) {
		for (sbits = 12; sbits <= 30; sbits++) {
			u64 addr = (counter<<12) | (sbits<<42);
			if (counter > ctr_max && (addr & 0xfffff))
				continue;
			u64 start_address, size, access_rights, page_size, flags;
			result = lv1_query_logical_partition_address_region_info(addr,
				&start_address, &size, &access_rights, &page_size, &flags);
			if (result == 0 && start_address != prev) {
				//printf("%012lx: size %7lx rights %lx pagesize %ld (%6x) flags 0x%016lx ", start_address, size, access_rights, page_size, 1<<page_size, flags);
				result = lv1_release_memory(start_address);
				if (result == 0) {
					blocks++;
					total += size;
					//printf("CLEANED\n");
				} else if (flags != 0) {
					if ((flags>>60) == 0xc) {
						if (lv1_unmap_htab(start_address) == 0)
							;//printf("CLEANED ");
						//printf("HTAB\n");
					} else {
						//printf("IOMEM\n");
					}
				} else {
					//printf("ERROR\n");
				}
				prev = start_address;
			}
		}
	}

	//printf("Cleaned up %ld bytes in %d blocks\n", total, blocks);

	for (i=0; i<16; i++)
	if (lv1_destruct_virtual_address_space(i) == 0)
		;//printf("Destroyed VAS %d\n", i);

		//lv1_panic(1);
}

extern volatile u64 _thread1_release;

void kload(void);
void klaunch(void);

int main()
{

	lv2_cleanup();
	
	kload();
	_thread1_release = 1ULL;
	klaunch();

	return 0;
}

