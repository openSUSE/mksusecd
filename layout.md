##isohybrid images done right

'[isohybrid](http://www.syslinux.org/wiki/index.php?title=Isohybrid)'
describes an image that can be both used as dvd (it has a valid
[iso9660](https://en.wikipedia.org/wiki/ISO_9660) file system
at offset 0) and as disk image (it also has a valid partition table).

The partition table type (
[gpt](https://en.wikipedia.org/wiki/GUID_Partition_Table),
[mbr](https://en.wikipedia.org/wiki/Master_boot_record),
or even [apple](https://en.wikipedia.org/wiki/Apple_Partition_Map))
doesn't matter - the
iso fs leaves enough unassigned space at its start (32 kB) to accomodate any.

The catch is: Where should the data partition itself start? Obviously, at block 0
would be a choice. But such a partition would again enclose the partition
table itself and partitioning tools really don't like such a layout.

Also, if the image should be
[EFI bootable](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#Booting),
the EFI boot image needs its own
partition. You can of course create a partition pointing to the EFI image
included in the iso fs but then you get overlapping partitions
and partitioning tools will eat you for lunch - this time for sure.

So, we do it differently. For isohybrid images we aim to create an image that

1. looks like an iso fs **and**
2. has a valid (sane) partition table with
    - one partition containing the same fs as 1. and
    - (optionally) another partition containing an EFI boot image

###Using an iso9660 file system for the partition

To achieve this, two [mkisofs](https://software.opensuse.org/package/mkisofs) runs are needed.
To understand why the following works it's important to know that the iso fs layout starts with
fs meta data, followed by directory data, followed by file data.

Now imagine you manage to put valid iso meta/directory data into the first
data file: this basically means you have an iso fs (excluding this file) that doesn't start at offset 0
but at the offset of the first data file.

Let's see how this works in detail.

There are two special files in our iso fs we need below:

- `efi`: (optional) EFI (fat-) boot image file (full path on x86_64 is `boot/x86_64/efi`)
- `glump`: hidden(!) 2k-block containing a magic blob (as the file doesn't have a
directory entry, we need the magic to find it in the image)

`efi` must be the 1st file, `glump` the 2nd (mkisofs lets you force the file order
easily).

The fs layout looks like this after a 1st mkisofs run:

```
== 1st run ==
iso meta/dir 1
data 1
  efi 1
  glump 1
    magic 1
  other files 1
```

(Indentation is used to indicate 'contains'.)

So, `glump` contains the magic blob, and `data` consists of `efi`, `glump`,
and `other files`.

The numbers are a reminder of the run the data was generated.

Now we copy `iso meta/dir` ... `glump` (inclusive) into a new `glump` and run
mkisofs on the same tree again. We get this layout:

```
== 2nd run ==
ofs 0->   iso meta/dir 2
          data 2
part1->     efi 2
part2->     glump 2
              iso meta/dir 1
              data 1
                efi 1
                glump 1
                  magic 1
            other files 2
```

Note that `other files 1` and `other files 2` are identical (we didn't change
any data there).

So, the result is:

- a valid iso fs at offset 0
- a valid iso fs at `glump 2` == `iso meta/dir 1`

This means we can add a partition table where:

- part1 points to the EFI boot partition
- part2 points to a valid iso fs referencing the same files as via offset 0
- part1 and part2 don't overlap

Note that the EFI boot image exists twice - but `efi 1` is unused
([El Torito](https://en.wikipedia.org/wiki/El_Torito_%28CD-ROM_standard%29)
references `efi 2` for booting). It would technically be possible to exclude
it in the 1st run (`efi 1` would then be gone) without loss of
functionality. But this would mean the users see different things depending on
whether they mount at offset 0 compared to partition 2. So, let's keep it
and waste the few bytes.


###Using a fat file system for the partition

Why not have an iso fs at offset 0 but a differnt fs type in partition 2?

The [fat](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system) file system
has a similar structure than iso fs: first fs meta data, then (mixed)
directory and file data. But it's easy to disentangle directory and file data
and ensure directories are first. So we can achieve the same basic fs layout and
use the same trick as above again.

Let's start by running mkisofs:

```
== 1st run ==
iso meta/dir 1
data 1
  efi 1
  glump 1
    magic 1
  other files 1
```

Now we need to prepare the content of `glump`. This is a bit more tricky
than with the iso fs above - but possible.

Here are the details:

1. create a new fat fs, with desired size (enough for all files, at least)
2. choose cluster size 2k, so files are aligned as in the iso fs (which also uses 2k blocks)
3. but: fat meta data can still be non-2k aligned - for this we will later move the
start of the whole file system so the start of the file data is 2k-aligned
4. create all files in the order they are in the iso fs, but with size 0 -
this will take care of creating all needed directory entries
5. then truncate all files to the correct size
6. while we do this, check if there are hidden files in the iso fs that would destroy
the relative file alignment; if such a gap is detected, insert padding files into the fat fs
7. delete any inserted padding files
8. verify that the (relative) file offsets match the offsets in the iso fs
9. determine the alignment blocks needed (as explained in step 3.)
10. copy alignment blocks + fat fs up to the start of the file data into `glump`
11. add `efi` (full 2k-blocks) to `glump`, if it exists

Then, after a 2nd mkisofs run, the layout looks like this:

```
== 2nd run ==
ofs 0->   iso meta/dir 2
          data 2
part1->     efi 2
            glump 2
              alignment
part2->       fat meta/dir
              efi
            other files 2
```

So, the result is:

- a valid iso fs at offset 0
- a valid fat fs at `fat meta/dir`

This means we can add a partition table where:

- part1 points to the EFI boot partition
- part2 points to a valid fat fs referencing the same files as via offset 0
- part1 and part2 don't overlap

Note that the fat fs is in fact **writable** but modifying any files will disrupt the
iso fs side. This shouldn't cause any problems in real live as users
probably don't copy the image onto a usb stick, use it for a while, and
then later burn a dvd with it.
