/*
 * efi.c:
 *
 * code from kernel
 *
 */

static char rcsid[] = "$Id: efi.c,v 1.5 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: efi.c,v $
 * Revision 1.5  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"

#include "mbr.h"
#include "efi_partition.h"

#define printf printk


static uint32_t
efi_crc32 (const void *buf, unsigned long len)
{
  return (crc32_le (~0L, buf, len) ^ ~0L);
}


static uint32_t
read_lba (uint64_t lba, uint8_t * buffer, uint32_t count)
{
  uint32_t totalreadcount = 0;

  if (!buffer || lba > disk_last_sector ())
    return 0;

  while (count)
    {

      int copied = 512;
      char data[512];
      memset (data, 0, sizeof (data));
      if (disk_read (data, lba++))
        break;
      if (copied > count)
        copied = count;
      memcpy (buffer, data, copied);
      buffer += copied;
      totalreadcount += copied;
      count -= copied;
    }
  return totalreadcount;
}

static inline int
pmbr_part_valid (struct mbr_partition *part, uint64_t lastlba)
{
  if (part->sys_ind == EFI_PMBR_OSTYPE_EFI_GPT &&
      le32_to_cpu (part->start_sect) == 1UL)
    return 1;
  return 0;
}

static int
is_pmbr_valid (legacy_mbr * mbr, uint64_t lastlba)
{
  int i;
  if (!mbr || le16_to_cpu (mbr->signature) != MSDOS_MBR_SIGNATURE)
    return 0;
  for (i = 0; i < 4; i++)
    if (pmbr_part_valid (&mbr->partition_record[i], lastlba))
      return 1;
  return 0;
}

#define MAX_PTES 1024
#define PTE_MAX_SIZE (sizeof(gpt_entry)*MAX_PTES)

static int
read_gpt_entries (gpt_header * gpt, gpt_entry * pte)
{
  uint32_t count;

  if (!gpt)
    return 0;

  count = le32_to_cpu (gpt->num_partition_entries) *
    le32_to_cpu (gpt->sizeof_partition_entry);
  if (!count)
    return 0;

  if (count > PTE_MAX_SIZE)
    count = PTE_MAX_SIZE;

  memset (pte, 0, count);

  if (read_lba (le64_to_cpu (gpt->partition_entry_lba),
                (uint8_t *) pte, count) < count)
    {
      return 0;
    }
  return 1;
}


static int
read_gpt_header (uint64_t lba, gpt_header * gpt)
{
  memset (gpt, 0, sizeof (gpt_header));

  if (read_lba (lba, (uint8_t *) gpt,
                sizeof (gpt_header)) < sizeof (gpt_header))
    {
      return 0;
    }

  return 1;
}



static int
is_gpt_valid (uint64_t lba, gpt_header * gpt, gpt_entry * ptes)
{
  uint32_t crc, origcrc;
  uint64_t lastlba;

  if (!gpt || !ptes)
    return 0;
  if (!(read_gpt_header (lba, gpt)))
    return 0;

  /* Check the GUID Partition Table signature */
  if (le64_to_cpu (gpt->signature) != GPT_HEADER_SIGNATURE)
    {
      printf ("GUID Partition Table Header signature is wrong:"
              "%lld != %lld\n",
              (unsigned long long) le64_to_cpu (gpt->signature),
              (unsigned long long) GPT_HEADER_SIGNATURE);
      goto fail;
    }

  /* Check the GUID Partition Table CRC */
  origcrc = le32_to_cpu (gpt->header_crc32);
  gpt->header_crc32 = 0;
  crc =
    efi_crc32 ((const unsigned char *) (gpt), le32_to_cpu (gpt->header_size));

  if (crc != origcrc)
    {
      printf
        ("GUID Partition Table Header CRC is wrong: %x != %x\n",
         crc, origcrc);
      goto fail;
    }
  gpt->header_crc32 = cpu_to_le32 (origcrc);

  /* Check that the my_lba entry points to the LBA that contains
   * the GUID Partition Table */
  if (le64_to_cpu (gpt->my_lba) != lba)
    {
      printf ("GPT my_lba incorrect: %lld != %lld\n",
              (unsigned long long) le64_to_cpu (gpt->my_lba),
              (unsigned long long) lba);
      goto fail;
    }

  /* Check the first_usable_lba and last_usable_lba are
   * within the disk.
   */
  lastlba = disk_last_sector ();
  if (le64_to_cpu (gpt->first_usable_lba) > lastlba)
    {
      printf ("GPT: first_usable_lba incorrect: %lld > %lld\n",
              (unsigned long long) le64_to_cpu (gpt->first_usable_lba),
              (unsigned long long) lastlba);
      goto fail;
    }
  if (le64_to_cpu (gpt->last_usable_lba) > lastlba)
    {
      printf ("GPT: last_usable_lba incorrect: %lld > %lld\n",
              (unsigned long long) le64_to_cpu (gpt->last_usable_lba),
              (unsigned long long) lastlba);
      goto fail;
    }

  if (!(read_gpt_entries (gpt, ptes)))
    goto fail;

  /* Check the GUID Partition Entry Array CRC */
  crc = efi_crc32 ((const unsigned char *) (ptes),
                   le32_to_cpu (gpt->num_partition_entries) *
                   le32_to_cpu (gpt->sizeof_partition_entry));

  if (crc != le32_to_cpu (gpt->partition_entry_array_crc32))
    {
      printf ("GUID Partitition Entry Array CRC check failed.\n");
      goto fail_ptes;
    }

  /* We're done, all's well */
  return 1;

fail_ptes:
fail:
  return 0;
}




static inline int
is_pte_valid (const gpt_entry * pte, const uint64_t lastlba)
{
  if ((!efi_guidcmp (pte->partition_type_guid, NULL_GUID)) ||
      le64_to_cpu (pte->starting_lba) > lastlba ||
      le64_to_cpu (pte->ending_lba) > lastlba)
    return 0;
  return 1;
}

#define force_gpt 0

static void
compare_gpts (gpt_header * pgpt, gpt_header * agpt, uint64_t lastlba)
{
  int error_found = 0;
  if (!pgpt || !agpt)
    return;
  if (le64_to_cpu (pgpt->my_lba) != le64_to_cpu (agpt->alternate_lba))
    {
      printf ("GPT:Primary header LBA != Alt. header alternate_lba\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (pgpt->my_lba),
              (unsigned long long) le64_to_cpu (agpt->alternate_lba));
      error_found++;
    }
  if (le64_to_cpu (pgpt->alternate_lba) != le64_to_cpu (agpt->my_lba))
    {
      printf ("GPT:Primary header alternate_lba != Alt. header my_lba\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (pgpt->alternate_lba),
              (unsigned long long) le64_to_cpu (agpt->my_lba));
      error_found++;
    }
  if (le64_to_cpu (pgpt->first_usable_lba) !=
      le64_to_cpu (agpt->first_usable_lba))
    {
      printf ("GPT:first_usable_lbas don't match.\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (pgpt->first_usable_lba),
              (unsigned long long) le64_to_cpu (agpt->first_usable_lba));
      error_found++;
    }
  if (le64_to_cpu (pgpt->last_usable_lba) !=
      le64_to_cpu (agpt->last_usable_lba))
    {
      printf ("GPT:last_usable_lbas don't match.\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (pgpt->last_usable_lba),
              (unsigned long long) le64_to_cpu (agpt->last_usable_lba));
      error_found++;
    }
  if (efi_guidcmp (pgpt->disk_guid, agpt->disk_guid))
    {
      printf ("GPT:disk_guids don't match.\n");
      error_found++;
    }
  if (le32_to_cpu (pgpt->num_partition_entries) !=
      le32_to_cpu (agpt->num_partition_entries))
    {
      printf ("GPT:num_partition_entries don't match: "
              "0x%x != 0x%x\n",
              le32_to_cpu (pgpt->num_partition_entries),
              le32_to_cpu (agpt->num_partition_entries));
      error_found++;
    }
  if (le32_to_cpu (pgpt->sizeof_partition_entry) !=
      le32_to_cpu (agpt->sizeof_partition_entry))
    {
      printf ("GPT:sizeof_partition_entry values don't match: "
              "0x%x != 0x%x\n",
              le32_to_cpu (pgpt->sizeof_partition_entry),
              le32_to_cpu (agpt->sizeof_partition_entry));
      error_found++;
    }
  if (le32_to_cpu (pgpt->partition_entry_array_crc32) !=
      le32_to_cpu (agpt->partition_entry_array_crc32))
    {
      printf ("GPT:partition_entry_array_crc32 values don't match: "
              "0x%x != 0x%x\n",
              le32_to_cpu (pgpt->partition_entry_array_crc32),
              le32_to_cpu (agpt->partition_entry_array_crc32));
      error_found++;
    }
  if (le64_to_cpu (pgpt->alternate_lba) != lastlba)
    {
      printf
        ("GPT:Primary header thinks Alt. header is not at the end of the disk.\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (pgpt->alternate_lba),
              (unsigned long long) lastlba);
      error_found++;
    }

  if (le64_to_cpu (agpt->my_lba) != lastlba)
    {
      printf ("GPT:Alternate GPT header not at the end of the disk.\n");
      printf ("GPT:%lld != %lld\n",
              (unsigned long long) le64_to_cpu (agpt->my_lba),
              (unsigned long long) lastlba);
      error_found++;
    }

  if (error_found)
    printf ("GPT: Use GNU Parted to correct GPT errors.\n");
  return;
}

static int
find_valid_gpt (gpt_header ** gpt, gpt_entry ** ptes)
{
  int good_pgpt = 0, good_agpt = 0, good_pmbr = 0;
  static gpt_header pgpt, agpt;
  static gpt_entry pptes[MAX_PTES], aptes[MAX_PTES];
  legacy_mbr *legacymbr = NULL;
  uint64_t lastlba;
  if (!gpt || !ptes)
    return 0;

  lastlba = disk_last_sector ();

  if (!force_gpt)
    {
      /* This will be added to the EFI Spec. per Intel after v1.02. */
      static uint8_t legacymbr_alloc[sizeof (*legacymbr)];
      legacymbr = (legacy_mbr *) legacymbr_alloc;
      if (legacymbr)
        {
          memset (legacymbr, 0, sizeof (*legacymbr));
          read_lba (0, (uint8_t *) legacymbr, sizeof (*legacymbr));
          good_pmbr = is_pmbr_valid (legacymbr, lastlba);
          legacymbr = NULL;
        }
      if (!good_pmbr)
        goto fail;
    }

  good_pgpt = is_gpt_valid (GPT_PRIMARY_PARTITION_TABLE_LBA, &pgpt, pptes);
  if (good_pgpt)
    good_agpt = is_gpt_valid (le64_to_cpu (pgpt.alternate_lba), &agpt, aptes);
  if (!good_agpt && force_gpt)
    good_agpt = is_gpt_valid (lastlba, &agpt, aptes);

  /* The obviously unsuccessful case */
  if (!good_pgpt && !good_agpt)
    goto fail;

  compare_gpts (&pgpt, &agpt, lastlba);

  /* The good cases */
  if (good_pgpt)
    {
      *gpt = &pgpt;
      *ptes = pptes;
      if (!good_agpt)
        {
          printf ("Alternate GPT is invalid, " "using primary GPT.\n");
        }
      return 1;
    }
  else if (good_agpt)
    {
      *gpt = &agpt;
      *ptes = aptes;
      printf ("Primary GPT is invalid, using alternate GPT.\n");
      return 1;
    }

fail:
  *gpt = NULL;
  *ptes = NULL;
  return 0;
}



int
efi_find_partitions (struct partition *partitions, int *num)
{
  gpt_header *gpt = NULL;
  gpt_entry *ptes = NULL;
  int i;
  int n;

  if (!partitions)
    return -1;
  if (!num)
    return -1;

  n = *num;

  *num = 0;

  if (!find_valid_gpt (&gpt, &ptes) || !gpt || !ptes)
    {
      return -1;
    }

  printf ("GUID parition table valid:");

  for (i = 0; i < le32_to_cpu (gpt->num_partition_entries); i++)
    {
      if (!is_pte_valid (&ptes[i], disk_last_sector ()))
        continue;
      partitions[*num].start = le64_to_cpu (ptes[i].starting_lba);
      partitions[*num].size = (le64_to_cpu (ptes[i].ending_lba) -
                               le64_to_cpu (ptes[i].starting_lba) + 1ULL);

      printf (" part%d", *num);
      (*num)++;
      if (!n == *num)
        break;
    }
  printf ("\n");

  return 0;
}
