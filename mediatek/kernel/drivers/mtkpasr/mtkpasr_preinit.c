#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/memory.h>
#include <linux/printk.h>

#define CONFIG_MTKPASR_MINDIESIZE		(0x10000000)	/* 256MB */
#define MTKPASR_LESSRESTRICT_PFNS		(0x40000)	/* 1GB */

/* Per-RANK Size (In bytes. From SW view) */
static unsigned long dpdrank_size = 0;
/* Per-BANK Size (In bytes. From SW view) */
unsigned long pasrbank_size = 0;
/* Ranksize in pfns */
static unsigned long pasrrank_pfns = 0;
/* Banksize in pfns */
static unsigned long pasrbank_pfns = 0;
/* We can't guarantee HIGHMEM zone is bank alignment, so we need another variable to represent it. */
static unsigned long mtkpasr_pfn_start = 0;

/* Set pageblock's mobility */
extern void set_pageblock_mobility(struct page *page, int mobility);

/* From dram_overclock.c */
extern bool pasr_is_valid(void);
/* To confirm PASR is valid */
static inline bool could_do_mtkpasr(void)
{
	return pasr_is_valid();
}

/* 
 * We will set an offset on which active PASR will be imposed. 
 * This is done by setting those pages as MIGRATE_MTKPASR type.
 * It only takes effect on HIGHMEM zone now! 
 */
void __meminit init_mtkpasr_range(struct zone *zone)
{
	struct pglist_data *pgdat;
	unsigned long start_pfn;
	unsigned long end_pfn;
	unsigned long pfn_bank_alignment;
	unsigned long pfn;
	struct page *page;

	/* Check whether our platform supports PASR */
	if (!could_do_mtkpasr()) {
		return;
	}

	/* Indicate node */
	pgdat = zone->zone_pgdat;

#ifdef CONFIG_HIGHMEM
	/* Start from HIGHMEM zone if we have CONFIG_HIGHMEM defined. */
	zone = zone + ZONE_HIGHMEM;
#else
	zone = zone + ZONE_NORMAL;
#endif
	start_pfn = zone->zone_start_pfn;
	end_pfn = start_pfn + zone->spanned_pages;

	/* Setup basic information : dpdrank_size, pasrbank_size */
	dpdrank_size = roundup(pgdat->node_spanned_pages << PAGE_SHIFT, CONFIG_MTKPASR_MINDIESIZE) >> 1;	/* 2 ranks */
	pasrbank_size = dpdrank_size >> 3;									/* 8 segments */

	/* Initialize pasrrank_pfns & pasrbank_pfns */
	pasrrank_pfns = dpdrank_size >> PAGE_SHIFT;
	pasrbank_pfns = pasrbank_size >> PAGE_SHIFT;
	pfn_bank_alignment = pasrbank_pfns;

	/* Align MTKPASR start pfn to bank alignment! (Round Up) */
	start_pfn = (start_pfn + pfn_bank_alignment - 1) & ~(pfn_bank_alignment - 1);

	/* Indicate the beginning PASR pfn */
#ifdef CONFIG_HIGHMEM
	start_pfn += pasrbank_pfns;
#else
	start_pfn += zone->spanned_pages >> 2;
#endif

#ifdef CONFIG_MT_ENG_BUILD
	/* One more bank-shift if we are in eng build. */
	/* start_pfn += pasrbank_pfns; */
#endif
	/* Give devices with large DRAMs less restriction */
	if (zone->spanned_pages > MTKPASR_LESSRESTRICT_PFNS) {
		start_pfn += pasrbank_pfns;
	}

#ifdef CONFIG_MTKPASR_NO_LASTBANK
	/* Align MTKPASR end pfn to bank alignment! (Round Down) */
	end_pfn = end_pfn & ~(pfn_bank_alignment - 1);
#endif

	/* Out MTKPASR Start PFN */
	mtkpasr_pfn_start = start_pfn;
	
	/* Set page mobility to MIGRATE_MTKPASR */
	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		
		/* If invalid */
		if (!early_pfn_valid(pfn))
			continue;

		/* Set it as MIGRATE_MTKPASR */
		page = pfn_to_page(pfn);
		if(!(pfn & (pageblock_nr_pages - 1)))
			set_pageblock_mobility(page, MIGRATE_MTKPASR);
	}	
	
	printk(KERN_NOTICE "[MTKPASR] @@@@@@ Start_pfn[%lu] End_pfn[%lu] Rank size = [%lu] Bank size = [%lu] @@@@@@\n",
			start_pfn, end_pfn, dpdrank_size, pasrbank_size);
}

/* 
 * Helper of constructing Memory (Virtual) Rank & Bank Information -
 *
 * start_pfn 	  - Pfn of the 1st page in that pasr range (Should be bank-aligned)
 * end_pfn   	  - Pfn of the one after the last page in that pasr range (Should be bank-aligned)
 * 		    (A hole may exist between end_pfn & bank-aligned(last_valid_pfn))
 * banks_per_rank - Number of banks in a rank
 *
 * Return    - The number of memory (virtual) banks, -1 means no valid range for PASR
 */
int __init compute_valid_pasr_range(unsigned long *start_pfn, unsigned long *end_pfn, u32 *banks_per_rank, int *num_ranks)
{
	struct zone *zone;
	int num_banks;
	unsigned long rank_start_pfn, rank_end_pfn;
	
	/* Check whether our platform supports PASR */
	if (!could_do_mtkpasr()) {
		return -1;
	}

#ifdef CONFIG_HIGHMEM
	/* We only focus on HIGHMEM zone */
	zone = NODE_DATA(0)->node_zones + ZONE_HIGHMEM;
#else
	zone = NODE_DATA(0)->node_zones + ZONE_NORMAL;
#endif	

	/* SANITY CHECK - Is this zone empty? */
	if (!populated_zone(zone)) {
		return -1;
	}

	/* Set PASR range */
	*start_pfn = mtkpasr_pfn_start;
	*end_pfn = (zone->zone_start_pfn + zone->spanned_pages + pasrbank_pfns - 1) & ~(pasrbank_pfns - 1); // Round-up to be pasrbank_pfns aligned

#ifdef CONFIG_MTKPASR_NO_LASTBANK
	/* Shift end_pfn to exclude the final bank. (Only for 6592) */
	*end_pfn -= pasrbank_pfns;
#endif

	/* Compute number of banks */
	num_banks = (*end_pfn - *start_pfn)/pasrbank_pfns;
	if (num_banks < 0) {
		return -1;
	}

	/* Compute number of ranks */
	*num_ranks = 0;
	rank_start_pfn = (mtkpasr_pfn_start + pasrrank_pfns - 1) & ~(pasrrank_pfns - 1);
	rank_end_pfn = rank_start_pfn + pasrrank_pfns;
	while (rank_start_pfn < *end_pfn) {
		/* We find a rank */
		if (rank_end_pfn < *end_pfn) {
			*num_ranks += 1;
		}
		/* To find the next one */
		rank_start_pfn += pasrrank_pfns;
		rank_end_pfn = rank_start_pfn + pasrrank_pfns;
	}

	/* Compute banks & ranks */
	*banks_per_rank = dpdrank_size/pasrbank_size;

	return num_banks;
}

/*
 * The segment which the page with pfn 0 belongs to (for the 1st channel).
 */
int dram_segment_offset_ch0 = 0;
/*
 * Translate sw bank to physical dram segment.
 * This will output different translation results depends on what dram model our platform uses.
 * non-interleaving(1-channel) vs. interleaving(n-channel, n > 1)
 *
 * Now it only supports non-interleaving translation.
 * (Is there any fault mapping?? Bad segment offset??)
 */
u32 __init pasr_bank_to_segment(unsigned long start_pfn, unsigned long end_pfn)
{
	/* Non-interleaving */
	return (start_pfn - ARCH_PFN_OFFSET) / pasrbank_pfns + dram_segment_offset_ch0;

	/*
	 *  Symmetric Interleaving
	 *  segment = (start_pfn - CONFIG_MEMPHYS_OFFSET) / pasrbank_pfns + dram_segment_offset_ch0;
	 *  // Dual-Channel   (n+n)
	 *  return segment | (segment << 8);
	 *  // Triple-Channel (n+n+n)
	 *  return segment | (segment << 8) | (segment << 16);
	 *  // Quad-Channel   (n+n+n+n)
	 *  return segment | (segment << 8) | (segment << 16) | (segment << 24);	 
	 */
}
