# How to modify a SUSE Linux installation medium

Sometimes it's convenient to make small modifications to the regular SUSE installation media.

For this, `mksusecd` exists.

## Replace or add files

You can replace or add any file to the medium. Simply create a directory structure and put the file into it.
Then specify all sources to be merged on the command line. For example

```sh
mkdir -p /tmp/foo/boot/x86_64/loader/
cp some_suitable_picture.jpg /tmp/foo/boot/x86_64/loader/welcome.jpg
mksusecd --create foo.iso suse.iso /tmp/foo
```
will create a new image `foo.iso` from `suse.iso` that has a new startup picture.


## Change default boot options

If you always have to enter the same boot options for each installation, add them:

```sh
mksusecd --create foo.iso --boot "textmode=1 sshd=1 password=XXX" suse.iso
```

It is also possible to add a new boot entry instead of modifying the existing one:

```sh
mksusecd --create foo.iso --add-entry "XXX" --boot "sshd=1 password=XXX" suse.iso
```


## Change default repository

If you run your own installation server you might want a medium that connects to it per default. To change the
default network repository do, for example:

```sh
mksusecd --create foo.iso --net "https://example.com/suse" suse.iso
```


## Create small network iso

If you are regularly installing via network or need to hand out a small image with your modifications, create a network iso.
For this, simply add the `--nano` option:

```sh
mksusecd --create foo.iso --nano suse.iso
```


## Integrate driver updates

To seamlessly integrate any driver updates you want applied, add them to the initrd. `mksusecd` has special code for this. For example

```sh
mksusecd --create foo.iso --initrd bar.dud suse.iso
```

creates a new `foo.iso` that will load the driver update `bar.dud` without any extra user interaction.


## Include add-ons

If you have a collection of rpms you would like to be able to install you can have `mksusecd` create an add-on repository and put it on the medium:

```sh
mksusecd --create foo.iso --addon foo.rpm bar.rpm -- suse.iso
```


## Update kernel (and kernel modules)

If you need an updated kernel to be able to run the installation, it can get really tricky. `mksusecd` lets you rework the installation
system included on the medium so that a new kernel is used during the installation. You just need the new kernel packages:

```sh
mksusecd --create foo.iso --kernel kernel-default.rpm kernel-firmware.rpm -- suse.iso
```

If you need also KMP packages, add them, too.

`mksusecd` will try to include the same modules as in the original installation medium. If some modules are missing, it will show
the differences.

Sometimes just a module is missing in the installation system that you really need. It's possible to add it this way:

```sh
mksusecd --create foo.iso --kernel kernel-default.rpm kernel-firmware.rpm --modules bar,zap -- suse.iso
```

This replaces the kernel and adds modules `bar` and `zap`. Module dependencies will be automatically taken into account.

Note that the `--kernel` option does **not** change the kernel that is going to be installed! For this, either create a driver update
with the kernel rpms and integrate the driver update or create an add-on with the kernel packages.



## SLES 15 and later: modules and repositories

SLES 15 supports so-called 'modules' (not to be confused with kernel modules) which are basically repositories for different product components.

`mksusecd` lets you create a single medium containing the parts you need.

```sh
mksusecd --list-repos suse.iso extra.iso
```

shows the modules that are on the medium. Pick the modules you need and do, for example:

```sh
mksusecd --create foo.iso --include-repos SLES15,Basesystem-Module,HPC-Module suse.iso extra.iso
```
