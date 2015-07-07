This is a tool to create SUSE Linux installation ISOs, either from scratch or
by modifying existing ISOs. A simple use case is to regenerate a ISO to
include fixes, see below.


### Installation

Often you will need [mkdud][1] along with mksusecd. Both mksusecd and mkdud are
included in openSUSE Tumbleweed. So on openSUSE Tumbleweed installation is as
simple as

```
zypper in mksusecd mkdud
```

[1]: https://github.com/openSUSE/mkdud

### Simple use case

We have a patch for yast2-core that is needed during installation and the
final system, e.g. for an AutoYaST installation. The patch is included in
yast2-core-3.1.12-0.x86_64.rpm.

1. Login as root since mksusecd needs root permissions for temporary mounting of
   file systems.

2. Create a Driver Update Disk (DUD) from yast2-core.rpm:
    ```
    mkdud --create bug-free.dud --dist 13.2 yast2-core-3.1.12-0.x86_64.rpm
    ```

3. Create a new ISO from the original openSUSE 13.2 ISO and the DUD:
    ```
    mksusecd --create bug-free.iso --initrd bug-free.dud openSUSE-13.2-DVD-x86_64.iso
    ```

Now you can use bug-free.iso as a replacement for openSUSE-13.2-DVD-x86_64.iso.

